#include <stdio.h>

#include "vm/composition_machine.h"
#include "vm/machine/hdc.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    machineInit(&session);
    vhdcReset();
    vhdcRefresh();
    machineFinal(&session);
    puts("M5:T37:S1:HDC-BOUNDARY:OK");
    return 0;
}
