#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/hdd.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->hdd != &session.hdd_storage) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    puts("M5:T36:S1:HDD-AUTHORITY:OK");
    return 0;
}
