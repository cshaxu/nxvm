#include "boot_fixture.h"
#include "core/board-base/machine_board_state.h"
#include "core/board-base/fdc.h"
#include "../../board-at/support/boot_fixture.h"
#include "../../board-xt/support/boot_fixture.h"

lib_status test_board_boot_video(const core_machine_board_state *board,
    x86_video_bus_observation *observation)
{
    return core_machine_vadp_observe_bus(board->shared_vadp, observation);
}

lib_u8 test_board_boot_pit_output(const core_machine_board_state *board,
    lib_u8 channel)
{
    return x86_pit_get_output(board->shared_pit, channel);
}

lib_u8 test_board_boot_fdc_dor(const core_machine_board_state *board)
{
    return board->fdc->data.dor;
}

void test_board_boot_bind_fdc_observer(core_machine_board_state *board,
    const core_machine_fdc_terminal_observation_provider *provider)
{
    board->fdc->connect.observation_provider = *provider;
}

x86_dma_signals test_board_boot_dma_signals(const core_machine_board_state *board,
    lib_u8 controller)
{
    return core_machine_dma_get_signals(board->shared_dma, controller);
}

lib_bool test_board_boot_bat_ready(const core_machine_board_state *board)
{
    return test_at_boot_bat_ready(board->shared_kbc);
}

lib_bool test_board_boot_keyboard_irq(const core_machine_board_state *board)
{
    return core_machine_pic_irq_source_is_asserted(board->keyboard_irq1_source);
}

lib_status test_board_boot_keyboard_repeat(const core_machine_board_state *board,
    lib_u64 *ticks)
{
    return test_at_boot_keyboard_repeat(board->shared_kbc, ticks);
}

lib_status test_board_boot_ppi_output(const core_machine_board_state *board,
    lib_u8 selector, x86_ppi8255_pins *pins)
{
    return test_xt_boot_ppi_output(board->xt_ppi_keyboard, selector, pins);
}

lib_status test_board_boot_xt_keyboard_event(const core_machine_board_state *board,
    lib_u64 *ticks)
{
    return x86_xt_keyboard_ticks_until_event(board->xt_keyboard, ticks);
}

lib_bool test_board_boot_xt_byte_ready(const core_machine_board_state *board)
{
    return test_xt_boot_byte_ready(board->xt_ppi_keyboard);
}

lib_u8 test_board_boot_pic_mask(const core_machine_board_state *board)
{
    lib_u8 mask = 0u;
    core_machine_pic_read_register(board->shared_pic_master, 1u, &mask);
    return mask;
}

core_machine_controller_timing_rule test_board_boot_pit_rule(
    const core_machine_board_state *board)
{
    return board->controller_timing.pit_clock;
}

lib_bool test_board_boot_refresh_request(core_machine_board_state *board,
    lib_u8 *address)
{
    return core_machine_board_refresh_request(board, address);
}
