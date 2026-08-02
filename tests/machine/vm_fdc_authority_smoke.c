#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/fdc.h"

int main(void)
{
    const vm_composition_live_machine *machine;

    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || &vfdc != machine->fdc) {
        machineFinal();
        return 1;
    }
    machineFinal();
    if (vm_machine_fdc_current() != NULL) return 1;
    puts("M5:T35:S1:FDC-AUTHORITY:OK");
    return 0;
}
