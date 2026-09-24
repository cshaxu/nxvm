#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/session_assets.h"
#include "app-nxvm/machine/machine_private.h"

#define VM_HOST_CANCELLATION_F9_SCAN_CODE 0x43u
#define VM_HOST_CANCELLATION_F9_VIRTUAL_KEY 0x78u

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    core_machine_guest_input_event event = { 0 };

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK) goto fail;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = VM_HOST_CANCELLATION_F9_SCAN_CODE;
    event.data.key.virtual_key = VM_HOST_CANCELLATION_F9_VIRTUAL_KEY;
    event.data.key.pressed = LIB_TRUE;
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_OK) goto fail;
    vm_machine_destroy(session);
    printf("M5:T201:S3:HOST-CANCELLATION:OK\n");
    return 0;

fail:
    vm_machine_destroy(session);
    return 1;
}
