#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/memory.h"
#include "support/core_machine_cpu_fixture.h"

static lib_i32 run_case(core_machine_cpu_profile profile)
{
    static const lib_u8 program[] = { 0xcdu, 0xf0u };
    static const lib_u8 ivt_entry[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    if (core_machine_create(&config, &machine) != LIB_STATUS_OK) goto fail;
    if (test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, sizeof(program)) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x03c0u, ivt_entry,
            sizeof(ivt_entry)) != LIB_STATUS_OK ||
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.linear_pc != 0x00000100u) {
        failed = 1;
    }

fail:
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80386);
    if (failed) return 1;
    printf("M5:T215:S1:CPU-INT-IVT:OK\n");
    return 0;
}
