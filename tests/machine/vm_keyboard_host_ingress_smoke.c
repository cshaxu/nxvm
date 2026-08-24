#include "type.h"

#include "core/machine/debug_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/platform/win32/win32.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

static C_INT vm_keyboard_host_ingress_read_byte(const vm_session *session,
    type_unsigned_16 offset, type_unsigned_8 *out_value)
{
    return core_machine_debug_read_memory(session->core_machine, offset,
        out_value, sizeof(*out_value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    core_platform_input_event event = {0};
    vm_platform_request request;
    type_unsigned_8 before;
    type_unsigned_8 after;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        !vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG0, &before)) goto fail;
    event.kind = CORE_PLATFORM_INPUT_KEY;
    event.data.key.scan_code = 0x2au;
    event.data.key.virtual_key = 0x10u;
    event.data.key.pressed = TYPE_TRUE;
    if (vm_session_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (!vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG0, &after) || after != before ||
        vm_platform_request_transport_dequeue_ingress(session->request_transport,
            &request) != TYPE_STATUS_OK ||
        request.kind != VM_PLATFORM_REQUEST_KEY_EVENT ||
        request.data.key_event.scan_code != 0x2au ||
        !request.data.key_event.pressed) goto fail;
    vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
        session->platform_run_handle, 0x2au, 0x10u, TYPE_TRUE);
    if (vm_platform_request_transport_dequeue_ingress(session->request_transport,
            &request) != TYPE_STATUS_OK ||
        request.kind != VM_PLATFORM_REQUEST_KEY_EVENT ||
        request.data.key_event.scan_code != 0x2au ||
        !request.data.key_event.pressed) goto fail;
    event.kind = (core_platform_input_kind)2;
    if (vm_session_submit_host_input(session, &event) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_platform_request_transport_dequeue_ingress(session->request_transport,
            &request) == TYPE_STATUS_OK) goto fail;
    vm_session_destroy(session);
    STD_PRINTF("M5:T226:S2:HOST-INGRESS:OK\n");
    return 0;

fail:
    event.kind = (core_platform_input_kind)2;
    if (vm_session_submit_host_input(session, &event) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_platform_request_transport_dequeue_ingress(session->request_transport,
            &request) == TYPE_STATUS_OK) goto fail;
    vm_session_destroy(session);
    return 1;
}
