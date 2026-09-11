#include "type.h"

#include "vm/machine/runtime/machine_interface.h"
#include "vm/machine/runtime/machine_private.h"
#include "../support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    core_machine_guest_input_event event = {0};

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK) goto fail;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = 0x2au;
    event.data.key.virtual_key = 0x10u;
    event.data.key.pressed = TYPE_TRUE;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (common_machine_observe_safe_point(session->executor) != LIB_STATUS_OK) goto fail;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (common_machine_observe_safe_point(session->executor) != LIB_STATUS_OK) goto fail;
    event.data.key.scan_code = 0x3bu;
    event.data.key.virtual_key = 0x70u;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (common_machine_observe_safe_point(session->executor) != LIB_STATUS_OK) goto fail;
    event.data.key.scan_code = 0x1eu;
    event.data.key.virtual_key = 'A';
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    event.data.key.pressed = TYPE_FALSE;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (common_machine_observe_safe_point(session->executor) != LIB_STATUS_OK ||
        common_machine_observe_safe_point(session->executor) != LIB_STATUS_OK) goto fail;
    event.kind = (core_machine_guest_input_kind)2;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_INVALID_ARGUMENT ||
        common_machine_observe_safe_point(session->executor) == LIB_STATUS_OK) goto fail;
    vm_machine_destroy(session);
    STD_PRINTF("M5:T226:S2:HOST-INGRESS:OK\n");
    return 0;

fail:
    event.kind = (core_machine_guest_input_kind)2;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_INVALID_ARGUMENT ||
        common_machine_observe_safe_point(session->executor) == LIB_STATUS_OK) goto fail;
    vm_machine_destroy(session);
    return 1;
}
