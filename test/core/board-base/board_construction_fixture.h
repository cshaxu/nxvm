#ifndef TEST_IBMPC_COMMON_BOARD_CONSTRUCTION_FIXTURE_H
#define TEST_IBMPC_COMMON_BOARD_CONSTRUCTION_FIXTURE_H
#include "core/board-base/machine_board_interface.h"

typedef lib_status (*test_core_constructor)(
    const core_machine_executor_config *config, void *owner,
    core_machine **out_machine);

/* Test-owned fault injection stays in the constructor's Core source.
 * Board owns production projection, attachment and its back-pointer check. */
lib_status test_board_construct(const core_machine_config *config,
    test_core_constructor construct, void *owner, core_machine **out_machine,
    core_machine_board_state **out_board);
#endif
