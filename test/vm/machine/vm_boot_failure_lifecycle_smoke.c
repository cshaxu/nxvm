#include "type.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/session_interface.h"
#include "../support/rom/session_assets.h"
#include "vm/composition/session/session_private.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_control_stop(&session->control);
    if (vm_session_control_is_running(&session->control)) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    STD_PRINTF("M5:T211:S3:BOOT-FAILURE-LIFECYCLE:OK\n");
    return 0;
}
