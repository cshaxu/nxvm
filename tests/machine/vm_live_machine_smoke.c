#include <stdio.h>

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    if ((&session) != NULL) return 1;
    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->cpu != &machine->cpu_storage ||
        machine->cpuins != &machine->cpuins_storage ||
        machine->ram != &machine->ram_storage ||
        machine->port != &machine->port_storage) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    if ((&session) != NULL) return 1;
    puts("M5:T23:S1:LIVE-MACHINE:OK");
    return 0;
}
