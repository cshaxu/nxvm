#include <stdio.h>

#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/qdx.h"

int main(void)
{
    machineInit();
    if (qdxTable[0xa2] == (t_faddrcc)NULL ||
        qdxTable[0xa3] == (t_faddrcc)NULL) {
        machineFinal();
        return 1;
    }
    machineFinal();
    puts("M5:T42:S1:DISK-FIRMWARE:OK");
    return 0;
}
