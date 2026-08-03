#include "type.h"



#include "core/machine/machine_interface.h"
#include "core/machine/debug_interface.h"
#include "../support/core_machine_executor_fixture.h"

static C_INT expect_status(ntvdm64_status actual, ntvdm64_status expected)
{
    return actual == expected ? 0 : 1;
}

static C_INT expect_lifecycle(
    core_machine *machine,
    core_machine_lifecycle expected)
{
    core_machine_lifecycle actual;

    if (core_machine_get_lifecycle(machine, &actual) != NTVDM64_STATUS_OK) {
        return 1;
    }

    return actual == expected ? 0 : 1;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run_result;
    core_machine_cpu_state cpu;
    core_machine_observation observation;
    C_UCHAR halt = 0xf4u;
    C_INT result = 0;

    result |= expect_status(test_core_machine_create_executor(0u, &machine),
                            NTVDM64_STATUS_OK);
    result |= expect_lifecycle(machine, CORE_MACHINE_INITIALIZED);
    result |= core_machine_configuration_memory_borrow(machine) == STD_NULL;
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            NTVDM64_STATUS_INVALID_STATE);

    result |= expect_status(core_machine_freeze_execution_providers(machine),
                            NTVDM64_STATUS_OK);
    result |= core_machine_configuration_memory_borrow(machine) != STD_NULL;

    result |= expect_status(core_machine_reset(machine), NTVDM64_STATUS_OK);
    result |= expect_lifecycle(machine, CORE_MACHINE_PAUSED);
    result |= expect_status(core_machine_get_cpu_state(machine, &cpu),
                            NTVDM64_STATUS_OK);
    result |= expect_status(core_machine_capture_observation(machine, &observation),
                            NTVDM64_STATUS_OK);
    result |= observation.lifecycle != CORE_MACHINE_PAUSED ||
              observation.cpu.cs != cpu.cs || observation.cpu.eip != cpu.eip;
    result |= core_machine_debug_memory_borrow(machine) == STD_NULL;
    result |= cpu.cs != 0xf000u || cpu.eip != 0x0000fff0u;
    result |= expect_status(core_machine_memory_write(machine, 0xffff0u, &halt, 1u),
                            NTVDM64_STATUS_OK);
    budget.instructions = 0u;
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            NTVDM64_STATUS_INVALID_ARGUMENT);

    budget.instructions = 1u;
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            NTVDM64_STATUS_OK);
    result |= run_result.reason != CORE_MACHINE_STOP_BUDGET;
    result |= expect_lifecycle(machine, CORE_MACHINE_PAUSED);

    result |= expect_status(core_machine_request_stop(machine),
                            NTVDM64_STATUS_OK);
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            NTVDM64_STATUS_OK);
    result |= run_result.reason != CORE_MACHINE_STOP_REQUESTED;
    result |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);
    result |= expect_status(core_machine_capture_observation(machine, &observation),
                            NTVDM64_STATUS_OK);
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            NTVDM64_STATUS_INVALID_STATE);

    result |= expect_status(core_machine_reset(machine), NTVDM64_STATUS_OK);
    result |= expect_status(core_machine_report_fault(machine, 0x1234u),
                            NTVDM64_STATUS_OK);
    result |= expect_lifecycle(machine, CORE_MACHINE_FAULTED);
    result |= expect_status(core_machine_capture_observation(machine, &observation),
                            NTVDM64_STATUS_OK);
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            NTVDM64_STATUS_FAULT);
    result |= run_result.reason != CORE_MACHINE_STOP_FAULT ||
              run_result.detail != 0x1234u ||
              run_result.linear_pc != 0xfffffff0u;

    core_machine_destroy(machine);
    if (result != 0) {
        return 1;
    }

    puts("M3:T2:S1:LIFECYCLE:OK");
    return 0;
}
