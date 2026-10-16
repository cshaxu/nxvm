#include "lib/types/types_interface.h"
#ifndef TEST_X86_NXVM_EMULATOR_EXECUTOR_FIXTURE_H
#define TEST_X86_NXVM_EMULATOR_EXECUTOR_FIXTURE_H

#include "x86-nxvm/board-base/machine_board_interface.h"

static lib_status test_board_create_executor(
    lib_size memory_bytes,
    x86_core_machine **out_machine)
{
    x86_nxvm_board_config config = { .memory_bytes = memory_bytes };
    lib_status status;

    status = x86_nxvm_board_create(&config, out_machine, LIB_NULL);
    if (status != LIB_STATUS_OK) {
        x86_core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
    }
    return status;
}

#endif
