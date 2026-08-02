#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/hdd.h"

int main(void)
{
    const vm_composition_live_machine *machine;

    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || &vhdd != machine->hdd) {
        machineFinal();
        return 1;
    }
    machineFinal();
    if (vm_machine_hdd_current() != NULL) return 1;
    puts("M5:T36:S1:HDD-AUTHORITY:OK");
    return 0;
}
