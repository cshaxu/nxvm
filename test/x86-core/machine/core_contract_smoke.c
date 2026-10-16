#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "x86-core/machine/machine_interface.h"
#include "executor_fixture.h"

lib_i32 main(void)
{
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    lib_u8 halt = 0xf4u;

    if (test_core_machine_create_executor(16u * 1024u * 1024u, &machine) != LIB_STATUS_OK) {
        return 1;
    }

    if (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return 2;
    }

    if (x86_core_machine_memory_write(machine, 0xffff0u, &halt, 1u) !=
            LIB_STATUS_OK ||
        x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 1u) {
        x86_core_machine_destroy(machine);
        return 3;
    }

    if (x86_core_machine_request_stop(machine) != LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return 5;
    }

    if (x86_core_machine_run(machine, budget, &result) !=
        LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return 6;
    }

    if (result.reason != X86_CORE_MACHINE_STOP_REQUESTED) {
        x86_core_machine_destroy(machine);
        return 7;
    }

    x86_core_machine_destroy(machine);
    lib_c_printf("CORE-CONTRACT:OK\n");
    return 0;
}
