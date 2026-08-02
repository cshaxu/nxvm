#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/qdx.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || qdxTable != machine->default_qdx->table) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    if (vm_profile_default_qdx_current() != NULL) return 1;
    puts("M5:T39:S1:QDX-AUTHORITY:OK");
    return 0;
}
