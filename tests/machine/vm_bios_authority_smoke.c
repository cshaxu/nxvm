#include <stdio.h>
#include <stdlib.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/bios.h"

int main(void)
{
    vm_composition_live_machine *session;
    const vm_composition_live_machine *machine;

    session = (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_initialize(session);
    machine = session;
    if (machine == NULL || machine->default_bios != &machine->default_bios_storage) {
        vm_composition_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_composition_finalize(session);
    STD_FREE(session);
    puts("M5:T38:S1:BIOS-AUTHORITY:OK");
    return 0;
}
