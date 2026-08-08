#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "../support/core_machine_cpu_fixture.h"

static C_INT run_case(core_machine_cpu_profile profile)
{
    static const C_UCHAR program[] = { 0xcdu, 0xf0u };
    static const C_UCHAR ivt_entry[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) goto fail;
    if (test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x03c0u, ivt_entry,
            sizeof(ivt_entry)) != TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.linear_pc != 0x00000100u) {
        failed = 1;
    }

fail:
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = 0;

    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80386);
    if (failed) return 1;
    STD_PRINTF("M5:T215:S1:CPU-INT-IVT:OK\n");
    return 0;
}
