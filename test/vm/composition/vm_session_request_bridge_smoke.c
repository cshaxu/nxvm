#include "type.h"

#include "vm/composition/session/request_bridge.h"

C_INT main(C_VOID)
{
    vm_session_request_bridge bridge;
    vm_session_request request;
    vm_session_request copy;
    STD_SIZE_T index;

    vm_session_request_bridge_initialize(&bridge);
    request.kind = VM_SESSION_REQUEST_KEY_EVENT;
    request.data.key_event.scan_code = 0x1eu;
    request.data.key_event.virtual_key = 0x41u;
    request.data.key_event.pressed = TYPE_TRUE;
    if (vm_session_request_bridge_enqueue(&bridge, &request) !=
        TYPE_STATUS_OK) return 1;
    request.data.key_event.scan_code = 0x30u;
    if (vm_session_request_bridge_dequeue(&bridge, &copy) !=
        TYPE_STATUS_OK ||
        copy.kind != VM_SESSION_REQUEST_KEY_EVENT ||
        copy.data.key_event.scan_code != 0x1eu ||
        copy.data.key_event.virtual_key != 0x41u ||
        copy.data.key_event.pressed != TYPE_TRUE) return 1;

    request.kind = VM_SESSION_REQUEST_KEY_EVENT;
    for (index = 0u; index < VM_SESSION_REQUEST_CAPACITY; ++index) {
        if (vm_session_request_bridge_enqueue(&bridge, &request) !=
            TYPE_STATUS_OK) return 1;
    }
    if (vm_session_request_bridge_enqueue(&bridge, &request) !=
        TYPE_STATUS_NO_MEMORY ||
        vm_session_request_bridge_dequeue(&bridge, &copy) !=
        TYPE_STATUS_OK ||
        vm_session_request_bridge_enqueue(&bridge, &request) !=
        TYPE_STATUS_OK ||
        vm_session_request_bridge_dequeue(&bridge, STD_NULL) !=
        TYPE_STATUS_INVALID_ARGUMENT) return 1;

    return 0;
}
