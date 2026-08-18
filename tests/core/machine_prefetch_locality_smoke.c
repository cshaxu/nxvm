#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

static C_INT run_halt(const core_machine_prefetch_locality_timing *timing,
    type_unsigned_64 *out_ticks)
{
    static const type_unsigned_8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u, 0xf4u
    };
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.prefetch_locality_timing = *timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){5u, 0u},
        &result) != TYPE_STATUS_OK;
    if (!failed && out_ticks != STD_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static C_INT locality_observer_contract(C_VOID)
{
    static const core_machine_prefetch_locality_timing locality = {2048u, 2u, 1u};
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_cpu_external_cycle_provider provider;
    C_VOID *context;
    C_INT failed = 0;

    config.prefetch_locality_timing = locality;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    provider = machine == STD_NULL ? STD_NULL :
        machine->executor_cpu_execution.external_cycle_provider;
    context = machine == STD_NULL ? STD_NULL :
        machine->executor_cpu_execution.external_cycle_context;
    failed |= provider == STD_NULL;
    if (!failed) {
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL, 0u, 1u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        failed |= machine->prefetch_locality_page_valid ||
            machine->prefetch_locality_round_ticks != 0u;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, 0u, 1u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, 1u, 1u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        failed |= !machine->prefetch_locality_page_valid ||
            machine->prefetch_locality_round_ticks != 3u;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        failed |= machine->prefetch_locality_page_valid ||
            machine->prefetch_locality_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

C_INT main(C_VOID)
{
    static const core_machine_prefetch_locality_timing disabled = {0u, 0u, 0u};
    static const core_machine_prefetch_locality_timing locality = {2048u, 2u, 1u};
    type_unsigned_64 baseline_ticks = 0u;
    type_unsigned_64 locality_ticks = 0u;
    C_INT failed = 0;

    failed |= !run_halt(&disabled, &baseline_ticks);
    failed |= !run_halt(&locality, &locality_ticks);
    /* The production path publishes a completed prefetch miss at retirement. */
    failed |= locality_ticks != baseline_ticks + 2u;
    failed |= !locality_observer_contract();
    if (failed != 0) return 1;
    STD_PRINTF("M5:T412:S1:EXTERNAL-READ-LOCALITY:OK\n");
    return 0;
}