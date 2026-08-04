#include "type.h"




#include "vm/composition/session/session.h"
#include "vm/composition/session/lifecycle.h"

#include "core/machine/debug_interface.h"

#include "vm/profile/default_profile/firmware/qdcga.h"


C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    vm_session_reset(session);
    vm_profile_default_cga_reset(&session->default_profile_context);
    {
        t_cpu *cpu = core_machine_debug_cpu_borrow(session->core_machine);
        if (cpu == STD_NULL) {
            vm_session_destroy(session);
            return 1;
        }
        cpu->data.ah = 0x00u;
        cpu->data.al = 0x03u;
    }
    vm_profile_default_cga_handle_int10(&session->default_profile_context);
    vm_session_destroy(session);
    puts("M5:T40:S1:QDCGA-BOUNDARY:OK");
    return 0;
}
