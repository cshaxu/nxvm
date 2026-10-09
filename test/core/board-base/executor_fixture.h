#include "lib/types/types_interface.h"
#ifndef TEST_IBMPC_COMMON_EXECUTOR_FIXTURE_H
#define TEST_IBMPC_COMMON_EXECUTOR_FIXTURE_H

#include "core/board-base/machine_board_interface.h"

static lib_status test_board_create_executor(
    lib_size memory_bytes,
    core_machine **out_machine)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    lib_status status;

    status = core_machine_create(&config, out_machine, LIB_NULL);
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
    }
    return status;
}

#endif
