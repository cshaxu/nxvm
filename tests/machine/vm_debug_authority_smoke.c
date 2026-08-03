#include "type.h"

#include <stdio.h>

#include <stdlib.h>


#include "vm/composition_live_machine.h"

#include "vm/composition_machine.h"

#include "vm/machine/debug.h"

C_INT main(C_VOID)
{
    vm_composition_live_machine *session;
    const vm_composition_live_machine *machine;

    session = (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_initialize(session);
    machine = session;
    if (machine == NULL || machine->debug != &machine->debug_storage) {
        vm_composition_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_machine_debug_set_trace(machine->debug, 2u);
    if (!machine->debug->data.flagTrace ||
        machine->debug->data.traceCount != 2u) {
        vm_composition_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_machine_debug_clear_trace(machine->debug);
    vm_composition_finalize(session);
    STD_FREE(session);
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
