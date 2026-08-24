#include "type.h"
#include "vm/composition/session/session_private.h"




#include "vm/composition/session/session_interface.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/machine/debug.h"

C_INT main(C_VOID)
{
    vm_session *session;
    vm_session *machine;

    session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    machine = session;
    if (machine == STD_NULL) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_machine_debug_set_trace(&machine->debug, 2u);
    if (!machine->debug.data.flagTrace ||
        machine->debug.data.traceCount != 2u) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_machine_debug_clear_trace(&machine->debug);
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
