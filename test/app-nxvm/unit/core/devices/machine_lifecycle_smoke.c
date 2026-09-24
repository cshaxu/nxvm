#include "lib/types/types_interface.h"
#include <stdio.h>



#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_executor_fixture.h"

static lib_i32 expect_status(lib_status actual, lib_status expected)
{
    return actual == expected ? 0 : 1;
}

static lib_i32 expect_lifecycle(
    core_machine *machine,
    core_machine_lifecycle expected)
{
    core_machine_lifecycle actual;

    if (core_machine_get_lifecycle(machine, &actual) != LIB_STATUS_OK) {
        return 1;
    }

    return actual == expected ? 0 : 1;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run_result;
    core_machine_cpu_state cpu;
    core_machine_observation observation;
    lib_u8 halt = 0xf4u;
    lib_i32 result = 0;

    result |= expect_status(test_core_machine_create_executor(0u, &machine),
                            LIB_STATUS_OK);
    result |= expect_lifecycle(machine, CORE_MACHINE_INITIALIZED);
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_INVALID_STATE);

    result |= expect_status(core_machine_freeze_execution_providers(machine),
                            LIB_STATUS_OK);

    result |= expect_status(core_machine_reset(machine), LIB_STATUS_OK);
    result |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);
    result |= expect_status(core_machine_get_cpu_state(machine, &cpu),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_capture_observation(machine, &observation),
                            LIB_STATUS_OK);
    result |= observation.lifecycle != CORE_MACHINE_STOPPED ||
              observation.cpu.cs != cpu.cs || observation.cpu.eip != cpu.eip;
    result |= cpu.cs != 0xf000u || cpu.eip != 0x0000fff0u;
    result |= expect_status(core_machine_memory_write(machine, 0xffff0u, &halt, 1u),
                            LIB_STATUS_OK);
    budget.instructions = 0u;
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_INVALID_ARGUMENT);

    budget.instructions = 1u;
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_OK);
    result |= run_result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    result |= expect_lifecycle(machine, CORE_MACHINE_PAUSED);

    result |= expect_status(core_machine_request_stop(machine),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_OK);
    result |= run_result.reason != CORE_MACHINE_STOP_REQUESTED;
    result |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);
    result |= expect_status(core_machine_capture_observation(machine, &observation),
                            LIB_STATUS_OK);
    result |= expect_lifecycle(machine, CORE_MACHINE_STOPPED);

    result |= expect_status(core_machine_reset(machine), LIB_STATUS_OK);
    result |= expect_status(core_machine_report_fault(machine, 0x1234u),
                            LIB_STATUS_OK);
    result |= expect_lifecycle(machine, CORE_MACHINE_FAULTED);
    result |= expect_status(core_machine_capture_observation(machine, &observation),
                            LIB_STATUS_OK);
    result |= expect_status(core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_INTERNAL_ERROR);
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
