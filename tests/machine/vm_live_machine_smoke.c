#include <stdio.h>

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

int main(void)
{
    const vm_composition_live_machine *machine;

    if (vm_composition_live_machine_current() != NULL) return 1;
    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || machine->cpu != &vcpu ||
        machine->cpuins != &vcpuins || machine->ram != &vram ||
        machine->port != &vport) {
        machineFinal();
        return 1;
    }
    machineFinal();
    if (vm_composition_live_machine_current() != NULL) return 1;
    puts("M5:T23:S1:LIVE-MACHINE:OK");
    return 0;
}
