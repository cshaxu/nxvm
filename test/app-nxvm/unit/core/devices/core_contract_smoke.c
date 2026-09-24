#include "lib/types/types_interface.h"
#include <stdio.h>



#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_executor_fixture.h"

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    lib_u8 halt = 0xf4u;

    if (test_core_machine_create_executor(0u, &machine) != LIB_STATUS_OK) {
        return 1;
    }

    if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 2;
    }

    if (core_machine_memory_write(machine, 0xffff0u, &halt, 1u) !=
            LIB_STATUS_OK ||
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 1u) {
        core_machine_destroy(machine);
        return 3;
    }

    if (core_machine_request_stop(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 5;
    }

    if (core_machine_run(machine, budget, &result) !=
        LIB_STATUS_OK) {
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
