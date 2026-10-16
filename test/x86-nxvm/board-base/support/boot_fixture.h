#ifndef TEST_BOARD_BOOT_FIXTURE_H
#define TEST_BOARD_BOOT_FIXTURE_H
#include "controller_fixture.h"
#include "composition_fixture.h"
#include "x86-nxvm/board-base/vadp_interface.h"
#include "x86-nxvm/chips/ppi8255/ppi8255_interface.h"

/* Diagnostic operations do not clock, reset or perform guest I/O. The
 * component owner reads its actual handles; callers receive copied values. */
lib_status test_board_boot_video(const x86_nxvm_board_state *board,
    x86_video_bus_observation *observation);
lib_u8 test_board_boot_pit_output(const x86_nxvm_board_state *board,
    lib_u8 channel);
lib_u8 test_board_boot_fdc_dor(const x86_nxvm_board_state *board);
void test_board_boot_bind_fdc_observer(x86_nxvm_board_state *board,
    const x86_nxvm_board_fdc_terminal_observation_provider *provider);
x86_dma_signals test_board_boot_dma_signals(const x86_nxvm_board_state *board,
    lib_u8 controller);
lib_bool test_board_boot_bat_ready(const x86_nxvm_board_state *board);
lib_bool test_board_boot_keyboard_irq(const x86_nxvm_board_state *board);
lib_status test_board_boot_keyboard_repeat(const x86_nxvm_board_state *board,
    lib_u64 *ticks);
lib_status test_board_boot_ppi_output(const x86_nxvm_board_state *board,
    lib_u8 selector, x86_ppi8255_pins *pins);
lib_status test_board_boot_xt_keyboard_event(const x86_nxvm_board_state *board,
    lib_u64 *ticks);
lib_bool test_board_boot_xt_byte_ready(const x86_nxvm_board_state *board);
lib_u8 test_board_boot_pic_mask(const x86_nxvm_board_state *board);
lib_bool test_board_boot_refresh_request(x86_nxvm_board_state *board,
    lib_u8 *address);
x86_nxvm_board_controller_timing_rule test_board_boot_pit_rule(
    const x86_nxvm_board_state *board);
#endif
