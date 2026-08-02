#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/cmos.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || &vcmos != machine->cmos) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    if (vm_machine_cmos_current() != NULL) return 1;
    puts("M5:T33:S1:CMOS-AUTHORITY:OK");
    return 0;
}
