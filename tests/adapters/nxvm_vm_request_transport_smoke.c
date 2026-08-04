#include "type.h"

#include "vm/platform/vm_request_transport.h"

typedef struct keyboard_state_observer {
    C_UINT count;
    uint32_t asynchronous_keys;
    uint32_t toggle_keys;
} keyboard_state_observer;

static C_VOID observe_keyboard_state(C_VOID *opaque,
                                   const vm_platform_request *request)
{
    keyboard_state_observer *observer = (keyboard_state_observer *)opaque;

    if (request->kind == VM_PLATFORM_REQUEST_KEYBOARD_STATE) {
        ++observer->count;
        observer->asynchronous_keys = request->data.keyboard_state.asynchronous_keys;
        observer->toggle_keys = request->data.keyboard_state.toggle_keys;
    }
}

C_INT main(C_VOID)
{
    vm_platform_request_transport transport;
    vm_platform_request request;
    vm_platform_request copy;
    keyboard_state_observer observer = {0u, 0u, 0u};
    STD_SIZE_T index;

    vm_platform_request_transport_initialize(&transport);
    request.kind = VM_PLATFORM_REQUEST_KEY_PRESS;
    request.data.key_press.scan_code = 0x1eu;
    request.data.key_press.virtual_key = 0x41u;
    if (vm_platform_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        TYPE_STATUS_OK ||
        vm_platform_request_transport_dequeue_ingress(&transport,
                                                             &copy) !=
        TYPE_STATUS_OK ||
        copy.data.key_press.scan_code != 0x1eu) return 1;

    vm_platform_request_transport_bind_consumer(
        &transport, observe_keyboard_state, &observer);
    request.kind = VM_PLATFORM_REQUEST_KEYBOARD_STATE;
    request.data.keyboard_state.asynchronous_keys =
        CORE_MACHINE_KEYBOARD_ASYNC_LEFT_SHIFT | CORE_MACHINE_KEYBOARD_ASYNC_CONTROL;
    request.data.keyboard_state.toggle_keys = CORE_MACHINE_KEYBOARD_TOGGLE_CAPS_LOCK;
    if (vm_platform_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        TYPE_STATUS_OK) return 1;
    vm_platform_request_transport_observe_execution_boundary(&transport);
    if (observer.count != 1u ||
        observer.asynchronous_keys != request.data.keyboard_state.asynchronous_keys ||
        observer.toggle_keys != request.data.keyboard_state.toggle_keys ||
        vm_platform_request_transport_execution_boundary_count(&transport) != 1u ||
        vm_platform_request_transport_dequeue_ingress(&transport,
                                                             &copy) !=
        TYPE_STATUS_UNSUPPORTED) return 1;

    request.kind = VM_PLATFORM_REQUEST_KEY_PRESS;
    request.data.key_press.scan_code = 0x30u;
    request.data.key_press.virtual_key = 0x42u;
    for (index = 0u; index < VM_PLATFORM_REQUEST_CAPACITY; ++index) {
        if (vm_platform_request_transport_enqueue_ingress(&transport,
                                                               &request) !=
            TYPE_STATUS_OK) return 1;
    }
    if (vm_platform_request_transport_enqueue_ingress(&transport,
                                                           &request) !=
        TYPE_STATUS_NO_MEMORY) return 1;

    vm_platform_request_transport_close(&transport);
    if (vm_platform_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        TYPE_STATUS_INVALID_STATE ||
        vm_platform_request_transport_dequeue_ingress(&transport,
                                                            &copy) !=
        TYPE_STATUS_OK ||
        copy.data.key_press.scan_code != 0x30u) return 1;

    vm_platform_request_transport_discard(&transport);
    if (vm_platform_request_transport_dequeue_ingress(&transport,
                                                            &copy) !=
        TYPE_STATUS_UNSUPPORTED) return 1;

    return 0;
}
