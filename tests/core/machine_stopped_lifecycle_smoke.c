#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_executor_fixture.h"

static C_INT expect_lifecycle(core_machine *machine,
    core_machine_lifecycle expected)
{
    core_machine_lifecycle actual;

    return core_machine_get_lifecycle(machine, &actual) != TYPE_STATUS_OK ||
        actual != expected;
}

static C_INT expect_reset_vector(core_machine *machine)
{
    core_machine_cpu_state cpu;

    return core_machine_get_cpu_state(machine, &cpu) != TYPE_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0x0000fff0u;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    C_INT failed = 0;

    failed |= test_core_machine_create_executor(0u, &machine) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);
    failed |= expect_reset_vector(machine);
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
    failed |= expect_lifecycle(machine, CORE_MACHINE_PAUSED);
    failed |= core_machine_request_stop(machine) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_REQUESTED;
    failed |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);
    failed |= expect_reset_vector(machine);

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    STD_PRINTF("M5:T170:S1:STOPPED-LIFECYCLE:OK\n");
    return 0;
}
