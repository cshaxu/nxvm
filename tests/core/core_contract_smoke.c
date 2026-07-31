#include <stdio.h>

#include "core/machine.h"

int main(void)
{
    nxvm_core_machine *machine = NULL;
    nxvm_core_machine_config config = {
        NXVM_CORE_ABI_VERSION,
        NXVM_CORE_PROFILE_TEST_MINIMAL,
        0u
    };
    nxvm_core_run_budget budget = { 1u, 0u };
    nxvm_core_run_result result;

    if (nxvm_core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK) {
        return 1;
    }

    if (nxvm_core_machine_reset(machine) != NXVM_CORE_STATUS_OK) {
        nxvm_core_machine_destroy(machine);
        return 2;
    }

    if (nxvm_core_machine_run(machine, budget, &result) !=
        NXVM_CORE_STATUS_UNSUPPORTED) {
        nxvm_core_machine_destroy(machine);
        return 3;
    }

    if (result.reason != NXVM_CORE_STOP_BUDGET || result.executed != 0u) {
        nxvm_core_machine_destroy(machine);
        return 4;
    }

    if (nxvm_core_machine_request_stop(machine) != NXVM_CORE_STATUS_OK) {
        nxvm_core_machine_destroy(machine);
        return 5;
    }

    if (nxvm_core_machine_run(machine, budget, &result) !=
        NXVM_CORE_STATUS_OK) {
        nxvm_core_machine_destroy(machine);
        return 6;
    }

    if (result.reason != NXVM_CORE_STOP_REQUESTED) {
        nxvm_core_machine_destroy(machine);
        return 7;
    }

    nxvm_core_machine_destroy(machine);
    puts("M3:T1:S1:CORE-CONTRACT:OK");
    return 0;
}
