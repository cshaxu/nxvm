#include "boot_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "x86-nxvm/board-base/fdc.h"
#include "../../board-at/support/boot_fixture.h"
#include "../../board-xt/support/boot_fixture.h"

lib_status test_board_boot_video(const x86_nxvm_board_state *board,
    x86_video_bus_observation *observation)
{
    return x86_nxvm_board_vadp_observe_bus(board->shared_vadp, observation);
}

lib_u8 test_board_boot_pit_output(const x86_nxvm_board_state *board,
    lib_u8 channel)
{
    return x86_pit_get_output(board->shared_pit, channel);
}

lib_u8 test_board_boot_fdc_dor(const x86_nxvm_board_state *board)
{
    return board->fdc->data.dor;
}

void test_board_boot_bind_fdc_observer(x86_nxvm_board_state *board,
    const x86_nxvm_board_fdc_terminal_observation_provider *provider)
{
    board->fdc->connect.observation_provider = *provider;
}

x86_dma_signals test_board_boot_dma_signals(const x86_nxvm_board_state *board,
    lib_u8 controller)
{
    return x86_nxvm_board_dma_get_signals(board->shared_dma, controller);
}

lib_bool test_board_boot_bat_ready(const x86_nxvm_board_state *board)
{
    return test_at_boot_bat_ready(board->shared_kbc);
}

lib_bool test_board_boot_keyboard_irq(const x86_nxvm_board_state *board)
{
    return x86_nxvm_board_pic_irq_source_is_asserted(board->keyboard_irq1_source);
}

lib_status test_board_boot_keyboard_repeat(const x86_nxvm_board_state *board,
    lib_u64 *ticks)
{
    return test_at_boot_keyboard_repeat(board->shared_kbc, ticks);
}

lib_status test_board_boot_ppi_output(const x86_nxvm_board_state *board,
    lib_u8 selector, x86_ppi8255_pins *pins)
{
    return test_xt_boot_ppi_output(board->xt_ppi_keyboard, selector, pins);
}

lib_status test_board_boot_xt_keyboard_event(const x86_nxvm_board_state *board,
    lib_u64 *ticks)
{
    return x86_xt_keyboard_ticks_until_event(board->xt_keyboard, ticks);
}

lib_bool test_board_boot_xt_byte_ready(const x86_nxvm_board_state *board)
{
    return test_xt_boot_byte_ready(board->xt_ppi_keyboard);
}

lib_u8 test_board_boot_pic_mask(const x86_nxvm_board_state *board)
{
    lib_u8 mask = 0u;
    x86_nxvm_board_pic_read_register(board->shared_pic_master, 1u, &mask);
    return mask;
}

x86_nxvm_board_controller_timing_rule test_board_boot_pit_rule(
    const x86_nxvm_board_state *board)
{
    return board->controller_timing.pit_clock;
}

lib_bool test_board_boot_refresh_request(x86_nxvm_board_state *board,
    lib_u8 *address)
{
    return x86_nxvm_board_refresh_request(board, address);
}
