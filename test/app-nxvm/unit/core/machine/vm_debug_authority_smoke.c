#include "lib/types/types_interface.h"
#include "type.h"
#include "app-nxvm/machine/machine_private.h"




#include "app-nxvm/machine/machine_interface.h"

#include "app-nxvm/machine/lifecycle.h"

#include "app-nxvm/machine/debug.h"

C_INT main(C_VOID)
{
    vm_machine *session;
    vm_machine *machine;

    session = ((vm_machine *)lib_allocate_zero(1u, sizeof(vm_machine)));
    if (session == LIB_NULL) return 1;
    vm_machine_initialize(session);
    machine = session;
    if (machine == LIB_NULL) {
        vm_machine_finalize(session);
        lib_release(session);
        return 1;
    }
    vm_machine_debug_set_trace(&machine->debug, 2u);
    if (!machine->debug.data.flagTrace ||
        machine->debug.data.traceCount != 2u) {
        vm_machine_finalize(session);
        lib_release(session);
        return 1;
    }
    vm_machine_debug_clear_trace(&machine->debug);
    vm_machine_finalize(session);
    lib_release(session);
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
