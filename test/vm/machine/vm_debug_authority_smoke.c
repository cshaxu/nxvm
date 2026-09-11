#include "type.h"
#include "vm/machine/runtime/machine_private.h"




#include "vm/machine/runtime/machine_interface.h"

#include "vm/machine/runtime/lifecycle.h"

#include "vm/machine/debug.h"

C_INT main(C_VOID)
{
    vm_machine *session;
    vm_machine *machine;

    session = ((vm_machine *)STD_CALLOC(1u, sizeof(vm_machine)));
    if (session == STD_NULL) return 1;
    vm_machine_initialize(session);
    machine = session;
    if (machine == STD_NULL) {
        vm_machine_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_machine_debug_set_trace(&machine->debug, 2u);
    if (!machine->debug.data.flagTrace ||
        machine->debug.data.traceCount != 2u) {
        vm_machine_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_machine_debug_clear_trace(&machine->debug);
    vm_machine_finalize(session);
    STD_FREE(session);
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
