#include "controller_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "../fdc_fixture.h"
#include "../hdc_fixture.h"
#include "../dma_fixture.h"

lib_bool test_board_fdc_finish_command_wait(x86_nxvm_board_state *board)
{
    return board != LIB_NULL && test_fdc_finish_command_wait(board->fdc);
}

lib_bool test_board_fdc_binding_matches(const x86_nxvm_board_state *board,
    const x86_core_machine *machine, x86_nxvm_board_media_id media_id, lib_u8 dma_channel)
{
    return !(board->fdc->connect.drives.media_id[0] != media_id ||
        board->fdc->connect.drives.media_id[1] != X86_NXVM_BOARD_MEDIA_ID_INVALID ||
        board->fdc->connect.dma_request.core_token == 0u ||
        board->fdc->connect.dma_request.channel != dma_channel ||
        board->fdc->connect.irq_source == LIB_NULL ||
        board->shared_pic_master == LIB_NULL || board->shared_pic_slave == LIB_NULL ||
        board->fdc->connect.machine != machine);
}

x86_nxvm_board_fdc_config test_board_fdc_connection_config(
    const x86_nxvm_board_state *board)
{
    return board->fdc->connect.config;
}

x86_nxvm_board_fdc_drive_bindings test_board_fdc_drive_bindings(
    const x86_nxvm_board_state *board)
{
    return board->fdc->connect.drives;
}

lib_bool test_board_fdc_advance_ticks(x86_nxvm_board_state *board, lib_u64 ticks)
{
    return test_fdc_advance_ticks(board->fdc, ticks);
}

void test_board_fdc_reset(x86_nxvm_board_state *board)
{
    x86_nxvm_board_fdc_reset(board->fdc);
}

void test_board_dma_transfers(x86_nxvm_board_state *board, x86_core_machine *machine,
    lib_u64 transfers, lib_u8 controllers)
{
    test_dma_transfers(board->shared_dma, machine, machine, transfers, controllers);
}

void test_board_fdc_advance(x86_nxvm_board_state *board)
{
    test_fdc_advance(board->fdc);
}

void test_board_fdc_refresh(x86_nxvm_board_state *board)
{
    x86_nxvm_board_fdc_refresh(board->fdc);
}

lib_bool test_board_fdc_interrupt_matches(const x86_nxvm_board_state *board,
    lib_bool asserted)
{
    return test_fdc_interrupt_matches(board->fdc, asserted);
}

lib_status test_board_fdc_capture(const x86_nxvm_board_state *board,
    x86_fdc_observation *observation)
{
    return x86_fdc_capture(board->fdc->chip, observation);
}

lib_u8 test_board_controller_scan_interrupt(x86_nxvm_board_state *board)
{
    return x86_nxvm_board_pic_scan_interrupt(board->shared_pic_master, board->shared_pic_slave);
}

lib_bool test_board_hdc_binding_is_present(const x86_nxvm_board_state *board)
{
    return !(board->hdc->connect.irq_source == LIB_NULL ||
        board->shared_pic_master == LIB_NULL || board->shared_pic_slave == LIB_NULL);
}

void test_board_hdc_service(x86_nxvm_board_state *board)
{
    hdc_service(board->hdc);
}

x86_hdc_observation test_board_hdc_observe(const x86_nxvm_board_state *board)
{
    return hdc_observe(board->hdc);
}

lib_u8 test_board_hdc_irq_pending(const x86_nxvm_board_state *board)
{
    return x86_nxvm_board_hdc_irq_pending(board->hdc);
}

x86_nxvm_board_media_id test_board_hdc_slave_media_id(const x86_nxvm_board_state *board)
{
    return board->hdc->connect.slave_media_id;
}

x86_nxvm_board_hdc_config test_board_hdc_connection_config(const x86_nxvm_board_state *board)
{
    return board->hdc->connect.config;
}

lib_bool test_board_fdc_advance_due(x86_nxvm_board_state *board)
{
    return test_fdc_advance_due(board->fdc);
}

lib_bool test_board_fdc_irq_source_is_asserted(const x86_nxvm_board_state *board)
{
    return x86_nxvm_board_pic_irq_source_is_asserted(board->fdc->connect.irq_source);
}
