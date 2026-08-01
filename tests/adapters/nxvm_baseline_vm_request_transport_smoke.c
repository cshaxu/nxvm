#include "adapters/nxvm_baseline/vm_request_transport.h"

int main(void)
{
    nxvm_baseline_vm_request_transport transport;
    nxvm_platform_vm_request request;
    nxvm_platform_vm_request copy;
    size_t index;

    nxvm_baseline_vm_request_transport_initialize(&transport);
    request.kind = NXVM_PLATFORM_VM_REQUEST_KEY_PRESS;
    request.data.key_press.scan_code = 0x1eu;
    request.data.key_press.virtual_key = 0x41u;
    if (nxvm_baseline_vm_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        NXVM_CORE_STATUS_OK ||
        nxvm_baseline_vm_request_transport_dequeue_egress(&transport,
                                                            &copy) !=
        NXVM_CORE_STATUS_UNSUPPORTED ||
        nxvm_baseline_vm_request_transport_dequeue_ingress(&transport,
                                                             &copy) !=
        NXVM_CORE_STATUS_OK ||
        copy.data.key_press.scan_code != 0x1eu) return 1;

    request.kind = NXVM_PLATFORM_VM_REQUEST_DISPLAY_MODE;
    request.data.window_display = 1;
    for (index = 0u; index < NXVM_PLATFORM_VM_REQUEST_CAPACITY; ++index) {
        if (nxvm_baseline_vm_request_transport_enqueue_egress(&transport,
                                                               &request) !=
            NXVM_CORE_STATUS_OK) return 1;
    }
    if (nxvm_baseline_vm_request_transport_enqueue_egress(&transport,
                                                           &request) !=
        NXVM_CORE_STATUS_NO_MEMORY) return 1;

    nxvm_baseline_vm_request_transport_close(&transport);
    if (nxvm_baseline_vm_request_transport_enqueue_ingress(&transport,
                                                            &request) !=
        NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_baseline_vm_request_transport_dequeue_egress(&transport,
                                                            &copy) !=
        NXVM_CORE_STATUS_OK ||
        copy.data.window_display != 1) return 1;

    nxvm_baseline_vm_request_transport_discard(&transport);
    if (nxvm_baseline_vm_request_transport_dequeue_egress(&transport,
                                                            &copy) !=
        NXVM_CORE_STATUS_UNSUPPORTED) return 1;

    return 0;
}
