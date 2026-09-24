#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_executor_fixture.h"

static lib_i32 expect_lifecycle(core_machine *machine,
    core_machine_lifecycle expected)
{
    core_machine_lifecycle actual;

    return core_machine_get_lifecycle(machine, &actual) != LIB_STATUS_OK ||
        actual != expected;
}

static lib_i32 expect_reset_vector(core_machine *machine)
{
    core_machine_cpu_state cpu;

    return core_machine_get_cpu_state(machine, &cpu) != LIB_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0x0000fff0u;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    lib_i32 failed = 0;

    failed |= test_core_machine_create_executor(0u, &machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);
    failed |= expect_reset_vector(machine);
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
    failed |= expect_lifecycle(machine, CORE_MACHINE_PAUSED);
    failed |= core_machine_request_stop(machine) != LIB_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_REQUESTED;
    failed |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);
    failed |= expect_reset_vector(machine);

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    printf("M5:T170:S1:STOPPED-LIFECYCLE:OK\n");
    return 0;
}
