#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "x86/core/trace_interface.h"
#include "app-nxvm/devices/machine_board_state.h"

lib_status core_machine_board_initialize_clocks(core_machine_board_state *board,
    const core_machine_clock_plan *plan)
{
    if (core_machine_clock_domain_initialize(&board->dma_clock,
            &plan->dma) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&board->pit_clock,
            &plan->pit) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&board->auxiliary_pit_clock,
            &plan->auxiliary_pit) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&board->rtc_clock,
            &plan->rtc) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&board->vadp_clock,
            &plan->vadp) != LIB_STATUS_OK ||
        core_machine_clock_domain_initialize(&board->kbc_clock,
            &plan->kbc) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}

void core_machine_board_reset_clocks(void *owner)
{
    core_machine_board_state *board = owner;
    core_machine_clock_domain_reset(&board->dma_clock);
    core_machine_clock_domain_reset(&board->pit_clock);
    core_machine_clock_domain_reset(&board->auxiliary_pit_clock);
    core_machine_clock_domain_reset(&board->rtc_clock);
    core_machine_clock_domain_reset(&board->vadp_clock);
    core_machine_clock_domain_reset(&board->kbc_clock);
}

lib_u64 core_machine_board_dma_ticks(void *owner, lib_u64 source_ticks)
{
    core_machine_board_state *board = owner;
    if (board == LIB_NULL) return 0u;
    return core_machine_clock_domain_advance(&board->dma_clock, source_ticks);
}

lib_bool core_machine_board_dma_request(void *owner)
{
    const core_machine_board_state *board = owner;
    if (board == LIB_NULL) return LIB_FALSE;
    return core_machine_dma_has_pending_request(&board->shared_dma_primary,
        &board->shared_dma_secondary) ? LIB_TRUE : LIB_FALSE;
}

void core_machine_board_dma_advance(void *owner, lib_u64 dma_ticks)
{
    core_machine_board_state *board = owner;
    if (board == LIB_NULL) return;
    core_machine_dma_advance_transaction(&board->shared_dma_latch,
        &board->shared_dma_primary, &board->shared_dma_secondary,
        board->core, dma_ticks);
}

core_machine_attachment_pit_ticks core_machine_board_pit_ticks_advance(void *owner,
    lib_u64 source_ticks)
{
    core_machine_board_state *board = owner;
    core_machine_attachment_pit_ticks ticks = {0u, 0u};
    if (board == LIB_NULL) return ticks;
    ticks.primary = core_machine_clock_domain_advance(&board->pit_clock,
        source_ticks);
    ticks.auxiliary = core_machine_clock_domain_advance(
        &board->auxiliary_pit_clock, source_ticks);
    return ticks;
}

void core_machine_board_pit_pic_advance(void *owner,
    core_machine_attachment_pit_ticks ticks)
{
    core_machine_board_state *board = owner;
    if (board == LIB_NULL) return;
    x86_pit_advance(board->shared_pit, ticks.primary);
    if (board->auxiliary_pit_configured) {
        x86_pit_advance(board->auxiliary_pit, ticks.auxiliary);
    }
    if (ticks.primary != 0u) {
        core_machine_trace_record(board->core, CORE_MACHINE_TRACE_PIT_ADVANCE,
            0u, (lib_u32)ticks.primary, 0u);
    }
    core_machine_pic_refresh(&board->shared_pic_master,
        &board->shared_pic_slave);
    core_machine_trace_record(board->core, CORE_MACHINE_TRACE_PIC_REFRESH,
        0u, 0u, 0u);
}

lib_bool core_machine_board_pic_pending(void *owner)
{
    core_machine_board_state *board = owner;
    if (board == LIB_NULL) return LIB_FALSE;
    return core_machine_pic_scan_interrupt(&board->shared_pic_master,
        &board->shared_pic_slave) ? LIB_TRUE : LIB_FALSE;
}

lib_u8 core_machine_board_pic_acknowledge(void *owner)
{
    core_machine_board_state *board = owner;
    if (board == LIB_NULL) return 0u;
    return core_machine_pic_get_interrupt(&board->shared_pic_master,
        &board->shared_pic_slave);
}

void core_machine_board_media_advance(void *owner, lib_u64 source_ticks,
    lib_u64 due_tick)
{
    core_machine_board_state *board = owner;

    if (board == LIB_NULL || source_ticks == 0u) return;
    if (board->fdc_configured) {
        core_machine_fdc_advance_at(&board->fdc, due_tick);
        core_machine_trace_record(board->core, CORE_MACHINE_TRACE_FDC_ADVANCE,
            0u, 0u, 0u);
    }
    if (board->hdc_configured) {
        core_machine_hdc_advance_at(&board->hdc, due_tick);
        core_machine_trace_record(board->core, CORE_MACHINE_TRACE_HDC_ADVANCE,
            0u, 0u, 0u);
    }
}

void core_machine_board_rtc_advance(void *owner, lib_u64 source_ticks)
{
    core_machine_board_state *board = owner;
    lib_u64 rtc_ticks;

    if (board == LIB_NULL || source_ticks == 0u) return;
    rtc_ticks = core_machine_clock_domain_advance(&board->rtc_clock, source_ticks);
    if (board->rtc_cmos_configured) {
        x86_rtc_advance(board->shared_rtc, rtc_ticks);
    }
    core_machine_trace_record(board->core, CORE_MACHINE_TRACE_RTC_ADVANCE,
        0u, (lib_u32)rtc_ticks, 0u);
}

/* Input and video follow Core's readiness phase. Presentation only consumes
 * copied snapshots outside this guest-time advancement. */
void core_machine_board_peripheral_advance(void *owner, lib_u64 source_ticks)
{
    core_machine_board_state *board = owner;
    lib_u64 kbc_ticks;
    lib_u64 vadp_ticks;

    if (board == LIB_NULL || source_ticks == 0u) return;
    kbc_ticks = core_machine_clock_domain_advance(&board->kbc_clock, source_ticks);
    if (board->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        x86_xt_keyboard_advance(board->xt_keyboard, source_ticks);
    } else {
        core_machine_kbc_advance(&board->shared_kbc, kbc_ticks);
    }
    core_machine_pic_advance(&board->shared_pic_master, &board->shared_pic_slave,
        source_ticks);
    core_machine_trace_record(board->core, CORE_MACHINE_TRACE_KBC_ADVANCE,
        0u, (lib_u32)kbc_ticks, 0u);
    vadp_ticks = core_machine_clock_domain_advance(&board->vadp_clock, source_ticks);
    x86_video_advance(board->shared_vadp.chip, vadp_ticks);
    core_machine_trace_record(board->core, CORE_MACHINE_TRACE_VADP_ADVANCE,
        0u, (lib_u32)vadp_ticks, 0u);
}
