#include <stdio.h>

#include "vm/composition_machine.h"
#include "vm/machine/vhdc.h"

int main(void)
{
    machineInit();
    vhdcReset();
    vhdcRefresh();
    machineFinal();
    puts("M5:T37:S1:HDC-BOUNDARY:OK");
    return 0;
}
