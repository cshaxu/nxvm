#include "kbc_state_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "../../board-at/support/state_fixture.h"

lib_i32 test_board_keyboard_repeat_cadence(x86_nxvm_board_state *board,
    lib_u64 initial_ticks, lib_u64 repeat_ticks)
{
    return test_keyboard_repeat_cadence(board->shared_kbc, initial_ticks, repeat_ticks);
}

lib_bool test_board_kbc_aux_enabled(const x86_nxvm_board_state *board)
{
    return test_kbc_aux_enabled(board->shared_kbc);
}

lib_bool test_board_keyboard_scanning(const x86_nxvm_board_state *board)
{
    return test_keyboard_scanning(board->shared_kbc);
}
