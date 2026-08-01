#include "vm/platform/request_bridge.h"

int main(void)
{
    nxvm_platform_vm_request_bridge bridge;
    nxvm_platform_vm_request request;
    nxvm_platform_vm_request copy;
    size_t index;

    nxvm_platform_vm_request_bridge_initialize(&bridge);
    request.kind = NXVM_PLATFORM_VM_REQUEST_KEY_PRESS;
    request.data.key_press.scan_code = 0x1eu;
    request.data.key_press.virtual_key = 0x41u;
    if (nxvm_platform_vm_request_bridge_enqueue(&bridge, &request) !=
        NXVM_CORE_STATUS_OK) return 1;
    request.data.key_press.scan_code = 0x30u;
    if (nxvm_platform_vm_request_bridge_dequeue(&bridge, &copy) !=
        NXVM_CORE_STATUS_OK ||
        copy.kind != NXVM_PLATFORM_VM_REQUEST_KEY_PRESS ||
        copy.data.key_press.scan_code != 0x1eu ||
        copy.data.key_press.virtual_key != 0x41u) return 1;

    request.kind = NXVM_PLATFORM_VM_REQUEST_STOP;
    for (index = 0u; index < NXVM_PLATFORM_VM_REQUEST_CAPACITY; ++index) {
        if (nxvm_platform_vm_request_bridge_enqueue(&bridge, &request) !=
            NXVM_CORE_STATUS_OK) return 1;
    }
    if (nxvm_platform_vm_request_bridge_enqueue(&bridge, &request) !=
        NXVM_CORE_STATUS_NO_MEMORY ||
        nxvm_platform_vm_request_bridge_dequeue(&bridge, &copy) !=
        NXVM_CORE_STATUS_OK ||
        nxvm_platform_vm_request_bridge_enqueue(&bridge, &request) !=
        NXVM_CORE_STATUS_OK ||
        nxvm_platform_vm_request_bridge_dequeue(&bridge, NULL) !=
        NXVM_CORE_STATUS_INVALID_ARGUMENT) return 1;

    return 0;
}
