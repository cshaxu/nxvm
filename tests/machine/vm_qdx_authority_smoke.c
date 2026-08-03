#include "type.h"

#include <stdio.h>

#include <stdlib.h>


#include "vm/composition_live_machine.h"

#include "vm/composition_machine.h"

#include "vm/profile/default_profile/firmware/qdx.h"

C_INT main(C_VOID)
{
    vm_composition_live_machine *session;
    const vm_composition_live_machine *machine;

    session = (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_composition_initialize(session);
    machine = session;
    if (machine == STD_NULL ||
        machine->default_qdx != &machine->default_qdx_storage ||
        machine->default_profile_context->qdx != machine->default_qdx ||
        core_machine_cpu_execution_context_extension(machine->cpu_execution) !=
            machine->default_profile_context) {
        vm_composition_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_composition_finalize(session);
    STD_FREE(session);
    puts("M5:T39:S1:QDX-AUTHORITY:OK");
    return 0;
}
