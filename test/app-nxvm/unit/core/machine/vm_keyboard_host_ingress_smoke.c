#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/rom/session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    core_machine_guest_input_event event = {0};

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK) goto fail;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = 0x2au;
    event.data.key.virtual_key = 0x10u;
    event.data.key.pressed = LIB_TRUE;
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_OK) goto fail;
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_OK) goto fail;
    event.data.key.scan_code = 0x3bu;
    event.data.key.virtual_key = 0x70u;
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_OK) goto fail;
    event.data.key.scan_code = 0x1eu;
    event.data.key.virtual_key = 'A';
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_OK) goto fail;
    event.data.key.pressed = LIB_FALSE;
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_OK) goto fail;
    event.kind = (core_machine_guest_input_kind)2;
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_INVALID_ARGUMENT) goto fail;
    vm_machine_destroy(session);
    printf("M5:T226:S2:HOST-INGRESS:OK\n");
    return 0;

fail:
    event.kind = (core_machine_guest_input_kind)2;
    if (vm_machine_submit_host_input(session, &event) != LIB_STATUS_INVALID_ARGUMENT) goto fail;
    vm_machine_destroy(session);
    return 1;
}
