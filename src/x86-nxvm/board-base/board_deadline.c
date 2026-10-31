#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "x86-nxvm/board-base/vadp.h"

static lib_u8 board_consider_relative(lib_u64 source_ticks, lib_u64 *io_source_ticks)
{
    if (source_ticks == 0u) return LIB_TRUE;
    if (*io_source_ticks == 0u || source_ticks < *io_source_ticks)
        *io_source_ticks = source_ticks;
    return LIB_FALSE;
}

static lib_u8 board_consider_clock(const x86_core_machine_clock_domain *clock,
    lib_u64 device_ticks, lib_u64 *io_source_ticks)
{
    lib_u64 source_ticks;

    if (device_ticks == 0u) return LIB_TRUE;
    if (x86_core_machine_clock_domain_source_ticks_until(clock, device_ticks,
            &source_ticks) != LIB_STATUS_OK) return LIB_FALSE;
    return board_consider_relative(source_ticks, io_source_ticks);
}

static lib_u8 board_consider_absolute(lib_u64 now, lib_u64 due_tick,
    lib_u64 *io_source_ticks)
{
    if (due_tick <= now) return LIB_TRUE;
    return board_consider_relative(due_tick - now, io_source_ticks);
}

static void board_consider_pit(const x86_pit *pit,
    const x86_core_machine_clock_domain *clock, lib_u64 *io_source_ticks)
{
    lib_u8 counter;
    lib_u64 nearest = 0u;

    for (counter = 0u; counter < 3u; ++counter) {
        lib_u64 device_ticks;

        if (x86_pit_ticks_until_output(pit, counter, &device_ticks) != LIB_STATUS_OK)
            continue;
        if (nearest == 0u || device_ticks < nearest) {
            nearest = device_ticks;
        }
    }
    if (nearest != 0u) (void)board_consider_clock(clock, nearest, io_source_ticks);
}

void x86_nxvm_board_deadline_observe(void *owner, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *out_observation)
{
    const x86_nxvm_board_state *board = owner;
    lib_u64 device_ticks;
    lib_u64 due_tick;
    lib_u8 dma_pending;
    lib_u8 dma_qualified;

    if (out_observation == LIB_NULL) return;
    *out_observation = (x86_core_machine_attachment_deadline_observation){0};
    if (board == LIB_NULL) return;
    if (board->refresh_hold_pending &&
        (out_observation->source_ticks == 0u || out_observation->source_ticks > 1u))
        out_observation->source_ticks = 1u;
    if (board->profile_binding.cpu_hold_asserted != LIB_NULL &&
        board->profile_binding.cpu_hold_asserted(board->profile_binding.context) &&
        (out_observation->source_ticks == 0u || out_observation->source_ticks > 1u))
        out_observation->source_ticks = 1u;
    dma_pending = x86_nxvm_board_dma_has_pending_request(
        board->shared_dma) ?
        LIB_TRUE : LIB_FALSE;
    dma_qualified = board->timing_declarations_copied &&
        board->dma_clock_explicit;
    if (dma_pending && !dma_qualified) {
        out_observation->l1_compatibility = LIB_TRUE;
        out_observation->fast_advance_blocked = LIB_TRUE;
    }

    /* A frozen fallback ratio remains an L2 timing claim, but is still a
     * Core-local conversion for a programmed PIT wake edge. */
    if (board->timing_declarations_copied && board->shared_vadp != LIB_NULL) {
        if (x86_video_ticks_until_event(board->shared_vadp->chip, &device_ticks) ==
                LIB_STATUS_OK && board_consider_clock(&board->vadp_clock,
                    device_ticks, &out_observation->source_ticks))
            out_observation->immediate_due = LIB_TRUE;
    }
    if (board->timing_declarations_copied) {
        board_consider_pit(board->shared_pit, &board->pit_clock,
            &out_observation->source_ticks);
        if (board->auxiliary_pit_configured)
            board_consider_pit(board->auxiliary_pit,
                &board->auxiliary_pit_clock, &out_observation->source_ticks);
    }
    if (board->timing_declarations_copied && board->rtc_cmos_configured &&
        x86_rtc_ticks_until_irq(board->shared_rtc, &device_ticks) ==
            LIB_STATUS_OK && board_consider_clock(&board->rtc_clock,
                device_ticks, &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (dma_pending && dma_qualified &&
        board_consider_clock(&board->dma_clock, 1u,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (x86_nxvm_board_fdc_next_due_tick(board->fdc, &due_tick) ==
        LIB_STATUS_OK) {
        if (due_tick <= now) out_observation->fast_advance_blocked = LIB_TRUE;
        if (board_consider_absolute(now, due_tick,
                &out_observation->source_ticks))
            out_observation->immediate_due = LIB_TRUE;
    }
    if (x86_nxvm_board_hdc_next_due_tick(board->hdc, &due_tick) ==
        LIB_STATUS_OK && board_consider_absolute(now, due_tick,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (board->profile_binding.next_deadline != LIB_NULL &&
        board->profile_binding.next_deadline(board->profile_binding.context, now, &due_tick) &&
        board_consider_absolute(now, due_tick,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (board->keyboard_topology != X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_NONE &&
        x86_nxvm_board_kbc_ticks_until_event(board->shared_kbc,
            &device_ticks) == LIB_STATUS_OK &&
        board_consider_clock(&board->kbc_clock, device_ticks,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (x86_nxvm_board_pic_ticks_until_event(board->shared_pic_master,
            board->shared_pic_slave, &device_ticks) == LIB_STATUS_OK &&
        board_consider_relative(device_ticks,
            &out_observation->source_ticks))
        out_observation->immediate_due = LIB_TRUE;
    if (board->keyboard_topology == X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI &&
        x86_xt_keyboard_ticks_until_event(board->xt_keyboard,
            &device_ticks) == LIB_STATUS_OK) {
        if (board_consider_relative(device_ticks, &out_observation->source_ticks))
            out_observation->immediate_due = LIB_TRUE;
    }
}
