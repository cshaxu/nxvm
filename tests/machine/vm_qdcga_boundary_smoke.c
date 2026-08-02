#include <stdio.h>

#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/qdcga.h"

int main(void)
{
    machineInit();
    qdcgaReset();
    machineFinal();
    puts("M5:T40:S1:QDCGA-BOUNDARY:OK");
    return 0;
}
