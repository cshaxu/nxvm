#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    vm_session *identity;
    vm_session_reset_vector vector;
    STD_SIZE_T bytes = 0u;
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != NTVDM64_STATUS_OK) return 1;
    identity = session;
    failed |= vm_session_reconfigure_memory(session, 32u * 1024u * 1024u) !=
        NTVDM64_STATUS_OK;
    failed |= session != identity;
    failed |= core_machine_get_memory_bytes(session->core_machine, &bytes) !=
        NTVDM64_STATUS_OK || bytes != 32u * 1024u * 1024u;
    failed |= vm_session_get_reset_vector(session, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u;
    vm_session_destroy(session);
    if (failed) return 1;
    puts("M5:T165:S1:SESSION-RECONFIGURE:OK");
    return 0;
}
