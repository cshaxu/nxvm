#include "type.h"

#include "vm/platform/request_bridge.h"



C_VOID nxvm_platform_vm_request_bridge_initialize(
    nxvm_platform_vm_request_bridge *bridge)
{
    if (bridge != STD_NULL) {
        STD_MEMSET(bridge, 0, sizeof(*bridge));
    }
}

ntvdm64_status nxvm_platform_vm_request_bridge_enqueue(
    nxvm_platform_vm_request_bridge *bridge,
    const nxvm_platform_vm_request *request)
{
    STD_SIZE_T index;

    if (bridge == STD_NULL || request == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (bridge->count == NXVM_PLATFORM_VM_REQUEST_CAPACITY) {
        return NTVDM64_STATUS_NO_MEMORY;
    }

    index = (bridge->head + bridge->count) % NXVM_PLATFORM_VM_REQUEST_CAPACITY;
    bridge->entries[index] = *request;
    ++bridge->count;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status nxvm_platform_vm_request_bridge_dequeue(
    nxvm_platform_vm_request_bridge *bridge,
    nxvm_platform_vm_request *out_request)
{
    if (bridge == STD_NULL || out_request == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (bridge->count == 0u) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    *out_request = bridge->entries[bridge->head];
    bridge->head = (bridge->head + 1u) % NXVM_PLATFORM_VM_REQUEST_CAPACITY;
    --bridge->count;
    return NTVDM64_STATUS_OK;
}
