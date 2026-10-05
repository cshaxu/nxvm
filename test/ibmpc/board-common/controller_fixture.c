#include "controller_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "fdc_fixture.h"
#include "hdc_fixture.h"
#include "dma_fixture.h"

lib_bool test_board_fdc_binding_matches(const core_machine_board_state *board,
    const core_machine *machine, core_machine_media_id media_id, lib_u8 dma_channel)
{
    return !(board->fdc->connect.drives.media_id[0] != media_id ||
        board->fdc->connect.drives.media_id[1] != CORE_MACHINE_MEDIA_ID_INVALID ||
        board->fdc->connect.dma_request.core_token == 0u ||
        board->fdc->connect.dma_request.channel != dma_channel ||
        board->fdc->connect.irq_source == LIB_NULL ||
        board->shared_pic_master == LIB_NULL || board->shared_pic_slave == LIB_NULL ||
        board->fdc->connect.machine != machine);
}

core_machine_fdc_config test_board_fdc_connection_config(
    const core_machine_board_state *board)
{
    return board->fdc->connect.config;
}

core_machine_fdc_drive_bindings test_board_fdc_drive_bindings(
    const core_machine_board_state *board)
{
    return board->fdc->connect.drives;
}

lib_bool test_board_fdc_advance_ticks(core_machine_board_state *board, lib_u64 ticks)
{
    return test_fdc_advance_ticks(board->fdc, ticks);
}

void test_board_fdc_reset(core_machine_board_state *board)
{
    core_machine_fdc_reset(board->fdc);
}

void test_board_dma_transfers(core_machine_board_state *board, core_machine *machine,
    lib_u64 transfers, lib_u8 controllers)
{
    test_dma_transfers(board->shared_dma, machine, machine, transfers, controllers);
}

void test_board_fdc_advance(core_machine_board_state *board)
{
    test_fdc_advance(board->fdc);
}

void test_board_fdc_refresh(core_machine_board_state *board)
{
    core_machine_fdc_refresh(board->fdc);
}

lib_bool test_board_fdc_interrupt_matches(const core_machine_board_state *board,
    lib_bool asserted)
{
    return test_fdc_interrupt_matches(board->fdc, asserted);
}

lib_status test_board_fdc_capture(const core_machine_board_state *board,
    x86_fdc_observation *observation)
{
    return x86_fdc_capture(board->fdc->chip, observation);
}

lib_u8 test_board_controller_scan_interrupt(core_machine_board_state *board)
{
    return core_machine_pic_scan_interrupt(board->shared_pic_master, board->shared_pic_slave);
}

lib_bool test_board_hdc_binding_is_present(const core_machine_board_state *board)
{
    return !(board->hdc->connect.irq_source == LIB_NULL ||
        board->shared_pic_master == LIB_NULL || board->shared_pic_slave == LIB_NULL);
}

void test_board_hdc_service(core_machine_board_state *board)
{
    hdc_service(board->hdc);
}

x86_hdc_observation test_board_hdc_observe(const core_machine_board_state *board)
{
    return hdc_observe(board->hdc);
}

lib_u8 test_board_hdc_irq_pending(const core_machine_board_state *board)
{
    return core_machine_hdc_irq_pending(board->hdc);
}

core_machine_media_id test_board_hdc_slave_media_id(const core_machine_board_state *board)
{
    return board->hdc->connect.slave_media_id;
}

core_machine_hdc_config test_board_hdc_connection_config(const core_machine_board_state *board)
{
    return board->hdc->connect.config;
}

lib_bool test_board_fdc_advance_due(core_machine_board_state *board)
{
    return test_fdc_advance_due(board->fdc);
}

lib_bool test_board_fdc_irq_source_is_asserted(const core_machine_board_state *board)
{
    return core_machine_pic_irq_source_is_asserted(board->fdc->connect.irq_source);
}
