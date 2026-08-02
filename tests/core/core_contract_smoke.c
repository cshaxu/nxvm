#include <stdio.h>

#include "core/machine/machine_interface.h"

int main(void)
{
    core_machine *machine = NULL;
    core_machine_config config = {
        CORE_MACHINE_PROFILE_TEST_MINIMAL,
        0u
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;

    if (core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK) {
        return 1;
    }

    if (core_machine_reset(machine) != NXVM_CORE_STATUS_OK) {
        core_machine_destroy(machine);
        return 2;
    }

    if (core_machine_run(machine, budget, &result) !=
        NXVM_CORE_STATUS_OK) {
        core_machine_destroy(machine);
        return 3;
    }

    if (result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u) {
        core_machine_destroy(machine);
        return 4;
    }

    if (core_machine_request_stop(machine) != NXVM_CORE_STATUS_OK) {
        core_machine_destroy(machine);
        return 5;
    }

    if (core_machine_run(machine, budget, &result) !=
        NXVM_CORE_STATUS_OK) {
        core_machine_destroy(machine);
        return 6;
    }

    if (result.reason != CORE_MACHINE_STOP_REQUESTED) {
        core_machine_destroy(machine);
        return 7;
    }

    core_machine_destroy(machine);
    puts("M3:T1:S1:CORE-CONTRACT:OK");
    return 0;
}
