#include <stdio.h>

#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/qdcga.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    machineInit(&session);
    qdcgaReset();
    machineFinal(&session);
    puts("M5:T40:S1:QDCGA-BOUNDARY:OK");
    return 0;
}
