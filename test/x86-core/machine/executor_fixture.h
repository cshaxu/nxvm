#include "lib/types/types_interface.h"
#ifndef TEST_X86_CORE_MACHINE_EXECUTOR_FIXTURE_H
#define TEST_X86_CORE_MACHINE_EXECUTOR_FIXTURE_H

#include "x86-core/machine/machine_interface.h"

static lib_status test_x86_core_machine_create_executor(
    lib_size memory_bytes,
    x86_core_machine **out_machine)
{
    x86_core_machine_executor_config config = {
        .memory_bytes = memory_bytes,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    lib_status status;

    status = x86_core_machine_neutral_create(&config, out_machine);
    if (status == LIB_STATUS_OK) {
        /* Synthetic RAM supplies the 386's high reset fetch for these tests. */
        const x86_core_machine_memory_alias_config alias = {
            0xffff0000u, 0x000f0000u, 0x00010000u
        };
        status = x86_core_machine_install_memory_aliases(*out_machine,
            &alias, 1u, LIB_FALSE);
    }
    if (status != LIB_STATUS_OK) {
        x86_core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
    }
    return status;
}

#endif
