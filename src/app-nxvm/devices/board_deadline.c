#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"

static lib_u8 board_consider_clock(const core_machine_clock_domain *clock,
    lib_u64 device_ticks, lib_u64 *io_source_ticks)
{
    lib_u64 source_ticks;

    if (device_ticks == 0u) return LIB_TRUE;
    if (core_machine_clock_domain_source_ticks_until(clock, device_ticks,
            &source_ticks) != LIB_STATUS_OK) return LIB_FALSE;
    if (*io_source_ticks == 0u || source_ticks < *io_source_ticks)
        *io_source_ticks = source_ticks;
    return LIB_FALSE;
}

static lib_u8 board_consider_absolute(lib_u64 now, lib_u64 due_tick,
    lib_u64 *io_source_ticks)
{
    lib_u64 source_ticks;

    if (due_tick <= now) return LIB_TRUE;
    source_ticks = due_tick - now;
    if (*io_source_ticks == 0u || source_ticks < *io_source_ticks)
        *io_source_ticks = source_ticks;
    return LIB_FALSE;
}

static lib_u8 board_consider_pit(const x86_pit *pit,
    const core_machine_clock_domain *clock, lib_u64 *io_source_ticks)
{
    lib_u8 counter;
    lib_u8 immediate_due = LIB_FALSE;

    for (counter = 0u; counter < 3u; ++counter) {
        lib_u64 device_ticks;

        if (x86_pit_ticks_until_output(pit, counter, &device_ticks) ==
            LIB_STATUS_OK && board_consider_clock(clock, device_ticks,
                io_source_ticks)) immediate_due = LIB_TRUE;
    }
    return immediate_due;
}

void core_machine_board_deadline_observe(void *owner, lib_u64 now,
    lib_bool timing_qualified,
    core_machine_board_deadline_observation *out_observation)
{
    const core_machine *machine = owner;
    lib_u64 device_ticks;
    lib_u64 due_tick;
    lib_u8 dma_pending;
    lib_u8 dma_qualified;

    if (out_observation == LIB_NULL) return;
    *out_observation = (core_machine_board_deadline_observation){0};
    if (machine == LIB_NULL) return;
    dma_pending = core_machine_dma_has_pending_request(
        &machine->board->shared_dma_primary, &machine->board->shared_dma_secondary) ?
        LIB_TRUE : LIB_FALSE;
    dma_qualified = timing_qualified &&
        machine->board->dma_clock_explicit;
    if (dma_pending && !dma_qualified) {
        out_observation->l1_compatibility = LIB_TRUE;
        out_observation->fast_advance_blocked = LIB_TRUE;
    }

    /* A frozen fallback ratio remains an L2 timing claim, but is still a
     * Core-local conversion for a programmed PIT wake edge. */
    if (timing_qualified) {
        if (board_consider_pit(machine->board->shared_pit.device, &machine->board->pit_clock,
                &out_observation->source_ticks))
            out_observation->immediate_due = LIB_TRUE;
        if (machine->board->auxiliary_pit_configured &&
            board_consider_pit(machine->board->auxiliary_pit.device,
                &machine->board->auxiliary_pit_clock, &out_observation->source_ticks))
            out_observation->immediate_due = LIB_TRUE;
    }
    if (timing_qualified && machine->board->rtc_cmos_configured &&
        x86_rtc_ticks_until_irq(machine->board->shared_rtc, &device_ticks) ==
            LIB_STATUS_OK && board_consider_clock(&machine->board->rtc_clock,
                device_ticks, &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (dma_pending && dma_qualified &&
        board_consider_clock(&machine->board->dma_clock, 1u,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (core_machine_fdc_next_due_tick(&machine->board->fdc, &due_tick) ==
        LIB_STATUS_OK) {
        if (due_tick <= now) out_observation->fast_advance_blocked = LIB_TRUE;
        if (board_consider_absolute(now, due_tick,
                &out_observation->source_ticks))
            out_observation->immediate_due = LIB_TRUE;
    }
    if (core_machine_hdc_next_due_tick(&machine->board->hdc, &due_tick) ==
        LIB_STATUS_OK && board_consider_absolute(now, due_tick,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (machine->board->d4_refresh_hold_pending &&
        board_consider_absolute(now, now + 1u,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (core_machine_kbc_ticks_until_event(&machine->board->shared_kbc,
            &device_ticks) == LIB_STATUS_OK &&
        board_consider_clock(&machine->board->kbc_clock, device_ticks,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (core_machine_pic_ticks_until_event(&machine->board->shared_pic_master,
            &machine->board->shared_pic_slave, &device_ticks) == LIB_STATUS_OK &&
        board_consider_absolute(now, now + device_ticks,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (machine->board->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI &&
        x86_xt_keyboard_ticks_until_event(machine->board->xt_keyboard,
            &device_ticks) == LIB_STATUS_OK) {
        if (device_ticks == 0u) out_observation->immediate_due = LIB_TRUE;
        else if (out_observation->source_ticks == 0u ||
            device_ticks < out_observation->source_ticks)
            out_observation->source_ticks = device_ticks;
    }
}
