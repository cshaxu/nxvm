#ifndef TEST_CORE_MACHINE_EXECUTOR_FIXTURE_H
#define TEST_CORE_MACHINE_EXECUTOR_FIXTURE_H

#include "core/machine/machine_interface.h"

static type_status test_core_machine_create_executor(
    STD_SIZE_T memory_bytes,
    core_machine **out_machine)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    type_status status;

    status = core_machine_create(&config, out_machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = STD_NULL;
    }
    return status;
}

#endif
