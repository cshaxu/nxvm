#include "type.h"




#include "vm/composition/session/lifecycle.h"

#include "core/machine/debug_interface.h"

#include "vm/profile/default_profile/firmware/qdcga.h"

#include "vm/profile/default_profile/firmware/qdx.h"

C_INT main(C_VOID)
{
    vm_session *session =
        (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    vm_session_reset(session);
    vm_profile_default_cga_reset(&session->default_profile_context);
    {
        t_cpu *cpu = core_machine_debug_cpu_borrow(session->core_machine);
        if (cpu == STD_NULL) {
            vm_session_finalize(session);
            STD_FREE(session);
            return 1;
        }
        cpu->data.ah = 0x00u;
        cpu->data.al = 0x03u;
    }
    if (session->default_qdx.table[0x10u] == STD_NULL) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    session->default_qdx.table[0x10u](&session->default_profile_context);
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T40:S1:QDCGA-BOUNDARY:OK");
    return 0;
}
