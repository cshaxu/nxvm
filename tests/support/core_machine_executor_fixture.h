#ifndef NTVDM64_TEST_CORE_MACHINE_EXECUTOR_FIXTURE_H
#define NTVDM64_TEST_CORE_MACHINE_EXECUTOR_FIXTURE_H

#include "core/machine/machine_interface.h"

static ntvdm64_status test_core_machine_create_executor(
    STD_SIZE_T memory_bytes,
    core_machine **out_machine)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    ntvdm64_status status;

    status = core_machine_create(&config, out_machine);
    if (status != NTVDM64_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = STD_NULL;
    }
    return status;
}

#endif
