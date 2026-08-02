#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/bios.h"

int main(void)
{
    const vm_composition_live_machine *machine;

    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || &vbios != machine->default_bios) {
        machineFinal();
        return 1;
    }
    machineFinal();
    if (vm_profile_default_bios_current() != NULL) return 1;
    puts("M5:T38:S1:BIOS-AUTHORITY:OK");
    return 0;
}
