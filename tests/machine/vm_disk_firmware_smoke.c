#include "type.h"




#include "vm/composition/session_lifecycle.h"

#include "vm/profile/default_profile/firmware/qdx.h"

C_INT main(C_VOID)
{
    vm_session *session =
        (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    if (session->default_qdx->table[0xa2] == STD_NULL ||
        session->default_qdx->table[0xa3] == STD_NULL) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T42:S1:DISK-FIRMWARE:OK");
    return 0;
}
