#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/debug.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->debug != &machine->debug_storage) {
        machineFinal(&session);
        return 1;
    }
    vm_machine_debug_set_trace(machine->debug, 2u);
    if (!machine->debug->data.flagTrace ||
        machine->debug->data.traceCount != 2u) {
        machineFinal(&session);
        return 1;
    }
    vm_machine_debug_clear_trace(machine->debug);
    machineFinal(&session);
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
