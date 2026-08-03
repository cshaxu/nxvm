#include <stdio.h>
#include <stdlib.h>

#include "core/machine/machine.h"
#include "vm/composition_live_machine.h"

int main(void)
{
    vm_composition_live_machine *machine = calloc(1u, sizeof(*machine));

    if (machine == NULL) return 1;
    vm_composition_live_machine_initialize(machine);
    if (machine->core_machine == NULL ||
        machine->cpu != core_machine_legacy_cpu_borrow(machine->core_machine) ||
        machine->cpuins != core_machine_legacy_cpu_instructions_borrow(
            machine->core_machine) ||
        machine->cpu_execution != core_machine_legacy_cpu_execution_borrow(
            machine->core_machine) ||
        machine->ram != core_machine_legacy_memory_borrow(machine->core_machine) ||
        machine->port != core_machine_legacy_port_borrow(machine->core_machine)) {
        vm_composition_live_machine_finalize(machine);
        free(machine);
        return 1;
    }
    vm_composition_live_machine_finalize(machine);
    free(machine);
    puts("M5:T83:S2:CORE-EXECUTOR-STORAGE:OK");
    return 0;
}
