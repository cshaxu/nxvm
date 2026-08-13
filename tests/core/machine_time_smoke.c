#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

static C_INT machine_time_expect(type_status status)
{
    return status == TYPE_STATUS_OK ? 0 : 1;
}

C_INT main(C_VOID)
{
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 2u, 0u };
    core_machine_run_result result;
    core_machine_observation observation;
    core_machine *machine = STD_NULL;
    const type_unsigned_8 nop = 0x90u;
    type_unsigned_64 elapsed = 0u;
    C_INT failed = 0;

    config.ticks_per_instruction = 3u;
    failed |= machine_time_expect(core_machine_create(&config, &machine));
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= machine_time_expect(core_machine_freeze_execution_providers(machine));
    failed |= machine_time_expect(core_machine_reset(machine));
    failed |= machine_time_expect(core_machine_get_elapsed_ticks(machine, &elapsed));
    failed |= elapsed != 0u;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, sizeof(nop)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff1u, &nop, sizeof(nop)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 2u ||
        result.ticks != 6u || result.elapsed_ticks != 6u;
    failed |= core_machine_capture_observation(machine, &observation) != TYPE_STATUS_OK;
    failed |= observation.elapsed_ticks != 6u;
    budget.instructions = 0u;
    budget.ticks = 3u;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 9u;
    failed |= machine_time_expect(core_machine_reset(machine));
    failed |= machine_time_expect(core_machine_get_elapsed_ticks(machine, &elapsed));
    failed |= elapsed != 0u;
    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T217:S2:TIME:OK\n");
    return 0;
}
