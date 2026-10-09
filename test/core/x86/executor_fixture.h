#include "lib/types/types_interface.h"
#ifndef TEST_CORE_MACHINE_EXECUTOR_FIXTURE_H
#define TEST_CORE_MACHINE_EXECUTOR_FIXTURE_H

#include "core/x86/machine_interface.h"

static lib_status test_core_machine_create_executor(
    lib_size memory_bytes,
    core_machine **out_machine)
{
    core_machine_executor_config config = { .memory_bytes = memory_bytes };
    lib_status status;

    status = core_machine_neutral_create(&config, out_machine);
    if (status == LIB_STATUS_OK) {
        /* Synthetic RAM supplies the 386's high reset fetch for these tests. */
        const core_machine_memory_alias_config alias = {
            0xffff0000u, 0x000f0000u, 0x00010000u
        };
        status = core_machine_install_memory_aliases(*out_machine,
            &alias, 1u, LIB_FALSE);
    }
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
    }
    return status;
}

#endif
