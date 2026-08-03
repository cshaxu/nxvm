#include <stdio.h>
#include <stdlib.h>

#include "vm/composition_machine.h"
#include "vm/profile/default_profile/firmware/qdx.h"

int main(void)
{
    vm_composition_live_machine *session =
        (vm_composition_live_machine *)calloc(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_initialize(session);
    if (session->default_qdx->table[0xa2] == NULL ||
        session->default_qdx->table[0xa3] == NULL) {
        vm_composition_finalize(session);
        free(session);
        return 1;
    }
    vm_composition_finalize(session);
    free(session);
    puts("M5:T42:S1:DISK-FIRMWARE:OK");
    return 0;
}
