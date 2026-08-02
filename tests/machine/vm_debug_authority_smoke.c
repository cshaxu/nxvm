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
    if (machine == NULL || &vdebug != machine->debug) {
        machineFinal(&session);
        return 1;
    }
    vm_machine_debug_set_trace(2u);
    if (!vdebug.data.flagTrace || vdebug.data.traceCount != 2u) {
        machineFinal(&session);
        return 1;
    }
    vm_machine_debug_clear_trace();
    machineFinal(&session);
    if (vm_machine_debug_current() != NULL) return 1;
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
