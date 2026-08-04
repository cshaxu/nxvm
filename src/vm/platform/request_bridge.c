#include "type.h"

#include "vm/platform/request_bridge.h"



C_VOID vm_platform_request_bridge_initialize(
    vm_platform_request_bridge *bridge)
{
    if (bridge != STD_NULL) {
        STD_MEMSET(bridge, 0, sizeof(*bridge));
    }
}

type_status vm_platform_request_bridge_enqueue(
    vm_platform_request_bridge *bridge,
    const vm_platform_request *request)
{
    STD_SIZE_T index;

    if (bridge == STD_NULL || request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (bridge->count == VM_PLATFORM_REQUEST_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }

    index = (bridge->head + bridge->count) % VM_PLATFORM_REQUEST_CAPACITY;
    bridge->entries[index] = *request;
    ++bridge->count;
    return TYPE_STATUS_OK;
}

type_status vm_platform_request_bridge_dequeue(
    vm_platform_request_bridge *bridge,
    vm_platform_request *out_request)
{
    if (bridge == STD_NULL || out_request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (bridge->count == 0u) {
        return TYPE_STATUS_UNSUPPORTED;
    }

    *out_request = bridge->entries[bridge->head];
    bridge->head = (bridge->head + 1u) % VM_PLATFORM_REQUEST_CAPACITY;
    --bridge->count;
    return TYPE_STATUS_OK;
}
