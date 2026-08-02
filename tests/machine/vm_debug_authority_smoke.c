#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/debug.h"

int main(void)
{
    const vm_composition_live_machine *machine;

    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || &vdebug != machine->debug) {
        machineFinal();
        return 1;
    }
    vm_machine_debug_set_trace(2u);
    if (!vdebug.data.flagTrace || vdebug.data.traceCount != 2u) {
        machineFinal();
        return 1;
    }
    vm_machine_debug_clear_trace();
    machineFinal();
    if (vm_machine_debug_current() != NULL) return 1;
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
