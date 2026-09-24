#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/session_assets.h"
#include "app-nxvm/machine/machine_private.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK) {
        vm_machine_destroy(session);
        return 1;
    }
    vm_machine_control_stop(&session->control);
    if (vm_machine_control_is_running(&session->control)) {
        vm_machine_destroy(session);
        return 1;
    }
    vm_machine_destroy(session);
    printf("M5:T211:S3:BOOT-FAILURE-LIFECYCLE:OK\n");
    return 0;
}
