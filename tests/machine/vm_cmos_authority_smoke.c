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
    if (machine == NULL || machine->cmos != &session.cmos_storage) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    puts("M5:T70:S1:CMOS-AUTHORITY:OK");
    return 0;
}
