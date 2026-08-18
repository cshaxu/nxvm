#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

static C_INT run_halt(const core_machine_external_memory_locality_timing *timing,
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
    config.external_memory_locality_timing = *timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){5u, 0u},
        &result) != TYPE_STATUS_OK;
    if (!failed && out_ticks != STD_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static C_INT run_write(const core_machine_external_memory_locality_timing *timing,
    type_unsigned_64 *out_ticks)
{
    static const type_unsigned_8 code[] = {
        0xb0u, 0x5au, 0xa2u, 0x10u, 0x00u, 0xf4u
    };
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    type_unsigned_8 value = 0u;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.external_memory_locality_timing = *timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){3u, 0u},
        &result) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(machine, 0x10u, &value, 1u) !=
        TYPE_STATUS_OK || value != 0x5au;
    if (!failed && out_ticks != STD_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}
static C_INT locality_observer_contract(C_VOID)
{
    static const core_machine_external_memory_locality_timing locality = {2048u, 2u, 1u};
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_cpu_external_cycle_provider provider;
    C_VOID *context;
    C_INT failed = 0;

    config.external_memory_locality_timing = locality;
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
        failed |= machine->external_memory_locality_page_valid ||
            machine->external_memory_locality_round_ticks != 0u;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, 0u, 1u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, 1u, 1u,
            TYPE_TRUE, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= !machine->external_memory_locality_page_valid ||
            machine->external_memory_locality_round_ticks != 3u;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        failed |= machine->external_memory_locality_page_valid ||
            machine->external_memory_locality_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

C_INT main(C_VOID)
{
    static const core_machine_external_memory_locality_timing disabled = {0u, 0u, 0u};
    static const core_machine_external_memory_locality_timing locality = {2048u, 2u, 1u};
    type_unsigned_64 baseline_ticks = 0u;
    type_unsigned_64 locality_ticks = 0u;
    type_unsigned_64 write_baseline_ticks = 0u;
    type_unsigned_64 write_locality_ticks = 0u;
    C_INT failed = 0;

    failed |= !run_halt(&disabled, &baseline_ticks);
    failed |= !run_halt(&locality, &locality_ticks);
    failed |= !run_write(&disabled, &write_baseline_ticks);
    failed |= !run_write(&locality, &write_locality_ticks);
    /* Retirement receives one initial prefetch miss and one write-page miss. */
    failed |= locality_ticks != baseline_ticks + 5u;
    failed |= write_locality_ticks != write_baseline_ticks + 4u;
    failed |= !locality_observer_contract();
    if (failed != 0) return 1;
    STD_PRINTF("M5:T412:S1:EXTERNAL-READ-LOCALITY:OK\n");
    STD_PRINTF("M5:T413:S1:EXTERNAL-WRITE-BRIDGE:OK\n");
    return 0;
}
