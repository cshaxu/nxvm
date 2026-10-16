#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-core/machine/trace_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"

lib_status x86_nxvm_board_initialize_clocks(x86_nxvm_board_state *board,
    const x86_nxvm_board_clock_plan *plan)
{
    if (x86_core_machine_clock_domain_initialize(&board->dma_clock,
            &plan->dma) != LIB_STATUS_OK ||
        x86_core_machine_clock_domain_initialize(&board->pit_clock,
            &plan->pit) != LIB_STATUS_OK ||
        x86_core_machine_clock_domain_initialize(&board->auxiliary_pit_clock,
            &plan->auxiliary_pit) != LIB_STATUS_OK ||
        x86_core_machine_clock_domain_initialize(&board->rtc_clock,
            &plan->rtc) != LIB_STATUS_OK ||
        x86_core_machine_clock_domain_initialize(&board->vadp_clock,
            &plan->vadp) != LIB_STATUS_OK ||
        x86_core_machine_clock_domain_initialize(&board->kbc_clock,
            &plan->kbc) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}

void x86_nxvm_board_reset_clocks(void *owner)
{
    x86_nxvm_board_state *board = owner;
    x86_core_machine_clock_domain_reset(&board->dma_clock);
    x86_core_machine_clock_domain_reset(&board->pit_clock);
    x86_core_machine_clock_domain_reset(&board->auxiliary_pit_clock);
    x86_core_machine_clock_domain_reset(&board->rtc_clock);
    x86_core_machine_clock_domain_reset(&board->vadp_clock);
    x86_core_machine_clock_domain_reset(&board->kbc_clock);
}

lib_u64 x86_nxvm_board_dma_ticks(void *owner, lib_u64 source_ticks)
{
    x86_nxvm_board_state *board = owner;
    if (board == LIB_NULL) return 0u;
    return x86_core_machine_clock_domain_advance(&board->dma_clock, source_ticks);
}

lib_bool x86_nxvm_board_dma_request(void *owner)
{
    const x86_nxvm_board_state *board = owner;
    if (board == LIB_NULL) return LIB_FALSE;
    return x86_nxvm_board_dma_has_pending_request(board->shared_dma) ? LIB_TRUE : LIB_FALSE;
}

void x86_nxvm_board_dma_advance(void *owner, lib_u64 dma_ticks)
{
    x86_nxvm_board_state *board = owner;
    if (board == LIB_NULL) return;
    x86_nxvm_board_dma_advance_transaction(board->shared_dma,
        board->core, dma_ticks);
}

void x86_nxvm_board_before_internal_advance(void *owner,
    lib_u64 source_ticks, lib_u64 due_tick)
{
    x86_nxvm_board_state *board = owner;
    lib_u64 primary_ticks;
    lib_u64 auxiliary_ticks;

    if (board == LIB_NULL || source_ticks == 0u) return;
    primary_ticks = x86_core_machine_clock_domain_advance(&board->pit_clock,
        source_ticks);
    auxiliary_ticks = x86_core_machine_clock_domain_advance(
        &board->auxiliary_pit_clock, source_ticks);
    x86_pit_advance(board->shared_pit, primary_ticks);
    if (board->auxiliary_pit_configured) {
        x86_pit_advance(board->auxiliary_pit, auxiliary_ticks);
    }
    if (primary_ticks != 0u) {
        x86_core_machine_trace_record(board->core, X86_NXVM_BOARD_TRACE_PIT_ADVANCE,
            0u, (lib_u32)primary_ticks, 0u);
    }
    x86_nxvm_board_pic_refresh(board->shared_pic_master,
        board->shared_pic_slave);
    x86_core_machine_trace_record(board->core, X86_NXVM_BOARD_TRACE_PIC_REFRESH,
        0u, 0u, 0u);

    /* Newly ready media cannot enter the PIC arbitration already completed at
     * this tick; preserve the board's existing readiness boundary. */
    if (board->fdc_configured) {
        x86_nxvm_board_fdc_advance_at(board->fdc, due_tick);
        x86_core_machine_trace_record(board->core, X86_NXVM_BOARD_TRACE_FDC_ADVANCE,
            0u, 0u, 0u);
    }
    if (board->hdc_configured) {
        x86_nxvm_board_hdc_advance_at(board->hdc, due_tick);
        x86_core_machine_trace_record(board->core, X86_NXVM_BOARD_TRACE_HDC_ADVANCE,
            0u, 0u, 0u);
    }
}

lib_bool x86_nxvm_board_pic_pending(void *owner)
{
    x86_nxvm_board_state *board = owner;
    if (board == LIB_NULL) return LIB_FALSE;
    return x86_nxvm_board_pic_scan_interrupt(board->shared_pic_master,
        board->shared_pic_slave) ? LIB_TRUE : LIB_FALSE;
}

lib_u8 x86_nxvm_board_pic_acknowledge(void *owner)
{
    x86_nxvm_board_state *board = owner;
    if (board == LIB_NULL) return 0u;
    return x86_nxvm_board_pic_get_interrupt(board->shared_pic_master,
        board->shared_pic_slave);
}

/* Input and video follow Core's readiness phase. Presentation only consumes
 * copied snapshots outside this guest-time advancement. */
void x86_nxvm_board_after_internal_advance(void *owner,
    lib_u64 source_ticks, lib_u64 due_tick)
{
    x86_nxvm_board_state *board = owner;
    lib_u64 rtc_ticks;
    lib_u64 kbc_ticks;
    lib_u64 vadp_ticks;

    (void)due_tick;
    if (board == LIB_NULL || source_ticks == 0u) return;
    rtc_ticks = x86_core_machine_clock_domain_advance(&board->rtc_clock, source_ticks);
    if (board->rtc_cmos_configured) {
        x86_rtc_advance(board->shared_rtc, rtc_ticks);
    }
    x86_core_machine_trace_record(board->core, X86_NXVM_BOARD_TRACE_RTC_ADVANCE,
        0u, (lib_u32)rtc_ticks, 0u);
    kbc_ticks = x86_core_machine_clock_domain_advance(&board->kbc_clock, source_ticks);
    if (board->keyboard_topology == X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI) {
        x86_xt_keyboard_advance(board->xt_keyboard, source_ticks);
    } else {
        x86_nxvm_board_kbc_advance(board->shared_kbc, kbc_ticks);
    }
    x86_nxvm_board_pic_advance(board->shared_pic_master, board->shared_pic_slave,
        source_ticks);
    x86_core_machine_trace_record(board->core, X86_NXVM_BOARD_TRACE_KBC_ADVANCE,
        0u, (lib_u32)kbc_ticks, 0u);
    vadp_ticks = x86_core_machine_clock_domain_advance(&board->vadp_clock, source_ticks);
    x86_nxvm_board_vadp_advance(board->shared_vadp, vadp_ticks);
    x86_core_machine_trace_record(board->core, X86_NXVM_BOARD_TRACE_VADP_ADVANCE,
        0u, (lib_u32)vadp_ticks, 0u);
}
