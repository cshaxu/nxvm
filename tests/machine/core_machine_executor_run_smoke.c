#include "type.h"



#include "core/machine/machine.h"

C_INT main(C_VOID)
{
    core_machine_config config = { .memory_bytes = 0u };
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    ntvdm64_type_unsigned_8 halt = 0xf4u;

    if (core_machine_create(&config, &machine) != NTVDM64_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    if (core_machine_freeze_execution_providers(machine) != NTVDM64_STATUS_OK ||
        core_machine_reset(machine) != NTVDM64_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_memory_write_physical(core_machine_configuration_memory_borrow(machine),
        0xffff0u, (ntvdm64_type_virtual_address)&halt, 1u);
    if (core_machine_run(machine, budget, &result) != NTVDM64_STATUS_OK ||
        result.executed != 1u || result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_run(machine, budget, &result) != NTVDM64_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    puts("M5:T83:S3:CORE-EXECUTOR-RUN:OK");
    return 0;
}
