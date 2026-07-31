#include <stdio.h>

#include "core/machine.h"

static int expect_status(nxvm_core_status actual, nxvm_core_status expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_lifecycle(
    nxvm_core_machine *machine,
    nxvm_core_machine_lifecycle expected)
{
    nxvm_core_machine_lifecycle actual;

    if (nxvm_core_machine_get_lifecycle(machine, &actual) != NXVM_CORE_STATUS_OK) {
        return 1;
    }

    return actual == expected ? 0 : 1;
}

int main(void)
{
    nxvm_core_machine *machine = NULL;
    nxvm_core_machine_config config = {
        NXVM_CORE_ABI_VERSION,
        NXVM_CORE_PROFILE_TEST_MINIMAL,
        0u
    };
    nxvm_core_run_budget budget = { 1u, 0u };
    nxvm_core_run_result run_result;
    nxvm_core_cpu_state cpu;
    int result = 0;

    result |= expect_status(nxvm_core_machine_create(&config, &machine),
                            NXVM_CORE_STATUS_OK);
    result |= expect_lifecycle(machine, NXVM_CORE_MACHINE_INITIALIZED);
    result |= expect_status(nxvm_core_machine_run(machine, budget, &run_result),
                            NXVM_CORE_STATUS_INVALID_STATE);

    result |= expect_status(nxvm_core_machine_reset(machine), NXVM_CORE_STATUS_OK);
    result |= expect_lifecycle(machine, NXVM_CORE_MACHINE_PAUSED);
    result |= expect_status(nxvm_core_machine_get_cpu_state(machine, &cpu),
                            NXVM_CORE_STATUS_OK);
    result |= cpu.cs != 0xf000u || cpu.eip != 0x0000fff0u;
    budget.instructions = 0u;
    result |= expect_status(nxvm_core_machine_run(machine, budget, &run_result),
                            NXVM_CORE_STATUS_INVALID_ARGUMENT);

    budget.instructions = 1u;
    result |= expect_status(nxvm_core_machine_run(machine, budget, &run_result),
                            NXVM_CORE_STATUS_OK);
    result |= run_result.reason != NXVM_CORE_STOP_BUDGET ||
              run_result.executed != 0u;
    result |= expect_lifecycle(machine, NXVM_CORE_MACHINE_PAUSED);

    result |= expect_status(nxvm_core_machine_request_stop(machine),
                            NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_run(machine, budget, &run_result),
                            NXVM_CORE_STATUS_OK);
    result |= run_result.reason != NXVM_CORE_STOP_REQUESTED;
    result |= expect_lifecycle(machine, NXVM_CORE_MACHINE_STOPPED);
    result |= expect_status(nxvm_core_machine_run(machine, budget, &run_result),
                            NXVM_CORE_STATUS_INVALID_STATE);

    result |= expect_status(nxvm_core_machine_reset(machine), NXVM_CORE_STATUS_OK);
    result |= expect_status(nxvm_core_machine_report_fault(machine, 0x1234u),
                            NXVM_CORE_STATUS_OK);
    result |= expect_lifecycle(machine, NXVM_CORE_MACHINE_FAULTED);
    result |= expect_status(nxvm_core_machine_run(machine, budget, &run_result),
                            NXVM_CORE_STATUS_FAULT);
    result |= run_result.reason != NXVM_CORE_STOP_FAULT ||
              run_result.detail != 0x1234u ||
              run_result.linear_pc != 0xfffffff0u;

    nxvm_core_machine_destroy(machine);
    if (result != 0) {
        return 1;
    }

    puts("M3:T2:S1:LIFECYCLE:OK");
    return 0;
}
