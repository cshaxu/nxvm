#include <stdio.h>
#include <stdlib.h>

#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/qdcga.h"

int main(void)
{
    vm_composition_live_machine *session =
        (vm_composition_live_machine *)calloc(1u, sizeof(*session));
    if (session == NULL) return 1;
    machineInit(session);
    vm_profile_default_cga_reset(session->default_profile_context);
    machineFinal(session);
    free(session);
    puts("M5:T40:S1:QDCGA-BOUNDARY:OK");
    return 0;
}
