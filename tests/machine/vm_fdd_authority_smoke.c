#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/fdd.h"

int main(void)
{
    const vm_composition_live_machine *machine;

    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || &vfdd != machine->fdd) {
        machineFinal();
        return 1;
    }
    machineFinal();
    if (vm_machine_fdd_current() != NULL) return 1;
    puts("M5:T34:S1:FDD-AUTHORITY:OK");
    return 0;
}
