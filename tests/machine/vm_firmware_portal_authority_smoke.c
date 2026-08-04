#include "type.h"




#include "vm/composition/session/session.h"

C_INT main(C_VOID)
{
    vm_session *session;
    vm_session *machine;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    machine = session;
    if (machine == STD_NULL ||
        machine->core_machine == STD_NULL ||
        vm_profile_default_context_execution(&machine->default_profile_context) == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T209:S3:FIRMWARE-PORTAL-AUTHORITY:OK");
    return 0;
}
