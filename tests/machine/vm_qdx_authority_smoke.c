#include "type.h"




#include "vm/composition/session.h"

#include "vm/composition/session_lifecycle.h"

#include "vm/profile/default_profile/firmware/qdx.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const vm_session *machine;

    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    machine = session;
    if (machine == STD_NULL ||
        machine->default_qdx != &machine->default_qdx_storage ||
        machine->default_profile_context->qdx != machine->default_qdx ||
        core_machine_cpu_execution_context_extension(
            vm_composition_machine_access_execution(session->core_access)) !=
            machine->default_profile_context) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T39:S1:QDX-AUTHORITY:OK");
    return 0;
}
