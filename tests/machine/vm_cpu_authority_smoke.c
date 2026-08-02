#include <stdio.h>

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    if (core_machine_cpu_current() != NULL ||
        core_machine_cpu_instructions_current() != NULL) {
        return 1;
    }

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->cpu != &machine->cpu_storage ||
        machine->cpuins != &machine->cpuins_storage ||
        machine->cpu != core_machine_cpu_current() ||
        machine->cpuins != core_machine_cpu_instructions_current() ||
        &vcpu != machine->cpu || &vcpuins != machine->cpuins) {
        machineFinal(&session);
        return 1;
    }

    machineFinal(&session);
    if (core_machine_cpu_current() != NULL ||
        core_machine_cpu_instructions_current() != NULL) {
        return 1;
    }

    puts("M5:T24:S1:CPU-AUTHORITY:OK");
    return 0;
}
