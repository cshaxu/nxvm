#include "vm/platform/vm_request_transport.h"

typedef struct keyboard_state_observer {
    unsigned count;
    uint32_t asynchronous_keys;
    uint32_t toggle_keys;
} keyboard_state_observer;

static void observe_keyboard_state(void *opaque,
                                   const nxvm_platform_vm_request *request)
{
    keyboard_state_observer *observer = (keyboard_state_observer *)opaque;

    if (request->kind == NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE) {
        ++observer->count;
        observer->asynchronous_keys = request->data.keyboard_state.asynchronous_keys;
        observer->toggle_keys = request->data.keyboard_state.toggle_keys;
    }
}

int main(void)
{
    nxvm_vm_request_transport transport;
    nxvm_platform_vm_request request;
    nxvm_platform_vm_request copy;
    keyboard_state_observer observer = {0u, 0u, 0u};
    size_t index;

    nxvm_vm_request_transport_initialize(&transport);
    request.kind = NXVM_PLATFORM_VM_REQUEST_KEY_PRESS;
    request.data.key_press.scan_code = 0x1eu;
    request.data.key_press.virtual_key = 0x41u;
    if (nxvm_vm_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        NTVDM64_STATUS_OK ||
        nxvm_vm_request_transport_dequeue_egress(&transport,
                                                            &copy) !=
        NTVDM64_STATUS_UNSUPPORTED ||
        nxvm_vm_request_transport_dequeue_ingress(&transport,
                                                             &copy) !=
        NTVDM64_STATUS_OK ||
        copy.data.key_press.scan_code != 0x1eu) return 1;

    nxvm_vm_request_transport_bind_consumer(
        &transport, observe_keyboard_state, &observer);
    request.kind = NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE;
    request.data.keyboard_state.asynchronous_keys =
        NXVM_KEYBOARD_ASYNC_LEFT_SHIFT | NXVM_KEYBOARD_ASYNC_CONTROL;
    request.data.keyboard_state.toggle_keys = NXVM_KEYBOARD_TOGGLE_CAPS_LOCK;
    if (nxvm_vm_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        NTVDM64_STATUS_OK) return 1;
    nxvm_vm_request_transport_observe_execution_boundary(&transport);
    if (observer.count != 1u ||
        observer.asynchronous_keys != request.data.keyboard_state.asynchronous_keys ||
        observer.toggle_keys != request.data.keyboard_state.toggle_keys ||
        nxvm_vm_request_transport_execution_boundary_count(&transport) != 1u ||
        nxvm_vm_request_transport_dequeue_ingress(&transport,
                                                             &copy) !=
        NTVDM64_STATUS_UNSUPPORTED) return 1;

    request.kind = NXVM_PLATFORM_VM_REQUEST_DISPLAY_MODE;
    request.data.window_display = 1;
    for (index = 0u; index < NXVM_PLATFORM_VM_REQUEST_CAPACITY; ++index) {
        if (nxvm_vm_request_transport_enqueue_egress(&transport,
                                                               &request) !=
            NTVDM64_STATUS_OK) return 1;
    }
    if (nxvm_vm_request_transport_enqueue_egress(&transport,
                                                           &request) !=
        NTVDM64_STATUS_NO_MEMORY) return 1;

    nxvm_vm_request_transport_close(&transport);
    if (nxvm_vm_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        NTVDM64_STATUS_INVALID_STATE ||
        nxvm_vm_request_transport_dequeue_egress(&transport,
                                                            &copy) !=
        NTVDM64_STATUS_OK ||
        copy.data.window_display != 1) return 1;

    nxvm_vm_request_transport_discard(&transport);
    if (nxvm_vm_request_transport_dequeue_egress(&transport,
                                                            &copy) !=
        NTVDM64_STATUS_UNSUPPORTED) return 1;

    return 0;
}
