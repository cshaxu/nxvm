#include "absent_memory_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"

lib_bool test_board_absent_memory_is_configured(core_machine_board_state *board)
{
    return board->absent_memory[0].configured;
}
