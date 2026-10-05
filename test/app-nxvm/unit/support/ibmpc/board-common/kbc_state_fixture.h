#ifndef TEST_BOARD_KBC_STATE_FIXTURE_H
#define TEST_BOARD_KBC_STATE_FIXTURE_H
#include "ibmpc/board-common/machine_board_interface.h"

lib_bool test_board_kbc_aux_enabled(const core_machine_board_state *board);
lib_bool test_board_keyboard_scanning(const core_machine_board_state *board);
lib_i32 test_board_keyboard_repeat_cadence(core_machine_board_state *board,
    lib_u64 initial_ticks, lib_u64 repeat_ticks);
#endif
