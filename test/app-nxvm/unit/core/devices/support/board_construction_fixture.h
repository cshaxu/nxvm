#ifndef TEST_BOARD_CONSTRUCTION_FIXTURE_H
#define TEST_BOARD_CONSTRUCTION_FIXTURE_H

#include "x86/core/machine.h"
#include "app-nxvm/devices/machine_board_state.h"

/* Fault injection is test-owned. The projection, neutral constructor and board
 * attachment are the same owners used by production; no alternate board setup. */
static lib_status test_core_machine_create_with_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_memory_test_allocation *memory_allocation,
    core_machine_port_test_allocation *port_allocation,
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
    status = core_machine_neutral_create_with_test_allocation(&executor,
        memory_allocation, port_allocation, &machine);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_board_create(machine, config, &board);
    if (status != LIB_STATUS_OK) return status;
    *out_machine = machine;
    if (out_board != LIB_NULL) *out_board = board;
    return LIB_STATUS_OK;
}

#endif
