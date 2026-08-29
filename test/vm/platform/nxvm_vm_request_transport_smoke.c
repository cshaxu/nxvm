#include "type.h"

#include "vm/platform/vm_request_transport.h"

typedef struct keyboard_event_observer {
    C_UINT count;
    vm_platform_request requests[3];
} keyboard_event_observer;

static C_VOID observe_keyboard_event(C_VOID *opaque,
                                   const vm_platform_request *request)
{
    keyboard_event_observer *observer = (keyboard_event_observer *)opaque;

    if (observer->count < sizeof(observer->requests) / sizeof(observer->requests[0])) {
        observer->requests[observer->count] = *request;
    }
    ++observer->count;
}

C_INT main(C_VOID)
{
    vm_platform_request_transport *transport = STD_NULL;
    vm_platform_request request;
    vm_platform_request copy;
    keyboard_event_observer observer = {0};
    STD_SIZE_T index;

    if (vm_platform_request_transport_create(&transport) != TYPE_STATUS_OK) return 1;
    request.kind = VM_PLATFORM_REQUEST_KEY_EVENT;
    request.data.key_event.scan_code = 0x1eu;
    request.data.key_event.virtual_key = 0x41u;
    request.data.key_event.pressed = 1;
    if (vm_platform_request_transport_enqueue_ingress(transport,
                                                            &request) !=
        TYPE_STATUS_OK ||
        vm_platform_request_transport_dequeue_ingress(transport,
                                                             &copy) !=
        TYPE_STATUS_OK ||
        copy.data.key_event.scan_code != 0x1eu || !copy.data.key_event.pressed) return 1;

    vm_platform_request_transport_bind_consumer(
        transport, observe_keyboard_event, &observer);
    request.kind = VM_PLATFORM_REQUEST_KEY_EVENT;
    request.data.key_event.scan_code = 0x2au;
    request.data.key_event.virtual_key = 0x10u;
    request.data.key_event.pressed = 1;
    if (vm_platform_request_transport_enqueue_ingress(transport,
                                                            &request) !=
        TYPE_STATUS_OK) return 1;
    request.data.key_event.scan_code = 0x1du;
    request.data.key_event.virtual_key = 0x11u;
    request.data.key_event.pressed = 1;
    if (vm_platform_request_transport_enqueue_ingress(transport,
                                                            &request) !=
        TYPE_STATUS_OK) return 1;
    request.data.key_event.scan_code = 0x30u;
    request.data.key_event.virtual_key = 0x42u;
    request.data.key_event.pressed = 0;
    if (vm_platform_request_transport_enqueue_ingress(transport,
                                                            &request) !=
        TYPE_STATUS_OK) return 1;
    vm_platform_request_transport_observe_execution_boundary(transport);
    vm_platform_request_transport_observe_execution_boundary(transport);
    vm_platform_request_transport_observe_execution_boundary(transport);
    if (observer.count != 3u ||
        observer.requests[0].kind != VM_PLATFORM_REQUEST_KEY_EVENT ||
        observer.requests[0].data.key_event.scan_code != 0x2au ||
        !observer.requests[0].data.key_event.pressed ||
        observer.requests[1].data.key_event.scan_code != 0x1du ||
        !observer.requests[1].data.key_event.pressed ||
        observer.requests[2].data.key_event.scan_code != 0x30u ||
        observer.requests[2].data.key_event.pressed ||
        vm_platform_request_transport_execution_boundary_count(transport) != 3u ||
        vm_platform_request_transport_dequeue_ingress(transport, &copy) !=
            TYPE_STATUS_UNSUPPORTED) return 1;

    request.kind = VM_PLATFORM_REQUEST_KEY_EVENT;
    request.data.key_event.scan_code = 0x30u;
    request.data.key_event.virtual_key = 0x42u;
    request.data.key_event.pressed = 1;
    for (index = 0u; index < VM_PLATFORM_REQUEST_CAPACITY; ++index) {
        if (vm_platform_request_transport_enqueue_ingress(transport,
                                                               &request) !=
            TYPE_STATUS_OK) return 1;
    }
    if (vm_platform_request_transport_enqueue_ingress(transport,
                                                           &request) !=
        TYPE_STATUS_NO_MEMORY) return 1;

    vm_platform_request_transport_close(transport);
    if (vm_platform_request_transport_enqueue_ingress(transport,
                                                            &request) !=
        TYPE_STATUS_INVALID_STATE ||
        vm_platform_request_transport_dequeue_ingress(transport,
                                                            &copy) !=
        TYPE_STATUS_OK ||
        copy.data.key_event.scan_code != 0x30u) return 1;

    vm_platform_request_transport_discard(transport);
    if (vm_platform_request_transport_dequeue_ingress(transport,
                                                            &copy) !=
        TYPE_STATUS_UNSUPPORTED) return 1;

    vm_platform_request_transport_destroy(transport);
    return 0;
}
