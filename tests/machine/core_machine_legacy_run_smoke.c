#include <stdio.h>

#include "core/machine/machine.h"

int main(void)
{
    core_machine_config config = {CORE_MACHINE_PROFILE_CUSTOM, 0u};
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    core_machine *machine = NULL;
    t_nubit8 halt = 0xf4u;

    if (core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK ||
        core_machine_enable_legacy_executor(machine) != NXVM_CORE_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_port_initialize(core_machine_legacy_port_borrow(machine));
    core_machine_memory_initialize(core_machine_legacy_memory_borrow(machine));
    core_machine_cpu_state_initialize(core_machine_legacy_cpu_execution_borrow(
        machine));
    core_machine_cpu_state_reset(core_machine_legacy_cpu_execution_borrow(machine));
    core_machine_memory_write_physical(core_machine_legacy_memory_borrow(machine),
        0xffff0u, (t_vaddrcc)&halt, 1u);
    if (core_machine_run(machine, budget, &result) != NXVM_CORE_STATUS_OK ||
        result.executed != 1u || result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_run(machine, budget, &result) != NXVM_CORE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        core_machine_memory_finalize(core_machine_legacy_memory_borrow(machine));
        core_machine_port_finalize(core_machine_legacy_port_borrow(machine));
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_memory_finalize(core_machine_legacy_memory_borrow(machine));
    core_machine_port_finalize(core_machine_legacy_port_borrow(machine));
    core_machine_destroy(machine);
    puts("M5:T83:S3:CORE-LEGACY-RUN:OK");
    return 0;
}
