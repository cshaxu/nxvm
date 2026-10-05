#include "board_construction_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"

lib_status test_board_construct(const core_machine_config *config,
    test_core_constructor construct, void *owner, core_machine **out_machine,
    core_machine_board_state **out_board)
{
    core_machine_executor_config executor;
    core_machine *machine;
    core_machine_board_state *board;
    lib_status status;

    if (out_board != LIB_NULL) *out_board = LIB_NULL;
    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    status = core_machine_board_prepare_executor(config, &executor);
    if (status != LIB_STATUS_OK) return status;
    status = construct(&executor, owner, &machine);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_board_create(machine, config, &board);
    if (status != LIB_STATUS_OK) return status;
    if (board->core != machine) {
        core_machine_destroy(machine);
        return LIB_STATUS_INTERNAL_ERROR;
    }
    *out_machine = machine;
    if (out_board != LIB_NULL) *out_board = board;
    return LIB_STATUS_OK;
}
