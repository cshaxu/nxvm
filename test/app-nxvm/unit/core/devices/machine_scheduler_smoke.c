#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct scheduler_provider_probe {
    lib_u32 advances;
    lib_u64 advanced_ticks;
} scheduler_provider_probe;

static void scheduler_provider_advance(void *opaque,
    lib_u64 elapsed_ticks)
{
    scheduler_provider_probe *probe = (scheduler_provider_probe *)opaque;

    if (probe != LIB_NULL) {
        ++probe->advances;
        probe->advanced_ticks += elapsed_ticks;
    }
}

static const core_machine_execution_provider scheduler_provider = {
    LIB_NULL,
    scheduler_provider_advance
};

lib_i32 main(void)
{
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 0u, 1u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    scheduler_provider_probe provider_probe = { 0u, 0u };
    const lib_u8 nop = 0x90u;
    lib_i32 failed = 0;

    config.ticks_per_instruction = 2u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= core_machine_bind_execution_provider(machine, &scheduler_provider,
        &provider_probe) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, sizeof(nop)) !=
        LIB_STATUS_OK;

    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || result.elapsed_ticks != 0u ||
        provider_probe.advances != 0u || provider_probe.advanced_ticks != 0u;

    budget.instructions = 1u;
    budget.ticks = 0u;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 3u ||
        provider_probe.advances != 1u || provider_probe.advanced_ticks != 3u;

    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T219:S2:SCHEDULER:OK\n");
    return 0;
}
