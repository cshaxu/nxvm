#include "type.h"

#include "vm/machine/executor_queue.h"



C_VOID vm_machine_executor_queue_initialize(
    vm_machine_executor_queue *bridge)
{
    if (bridge != STD_NULL) {
        STD_MEMSET(bridge, 0, sizeof(*bridge));
    }
}

type_status vm_machine_executor_queue_enqueue(
    vm_machine_executor_queue *bridge,
    const vm_machine_request *request)
{
    STD_SIZE_T index;

    if (bridge == STD_NULL || request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (bridge->count == VM_MACHINE_EXECUTOR_QUEUE_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }

    index = (bridge->head + bridge->count) % VM_MACHINE_EXECUTOR_QUEUE_CAPACITY;
    bridge->entries[index] = *request;
    ++bridge->count;
    return TYPE_STATUS_OK;
}

type_status vm_machine_executor_queue_dequeue(
    vm_machine_executor_queue *bridge,
    vm_machine_request *out_request)
{
    if (bridge == STD_NULL || out_request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (bridge->count == 0u) {
        return TYPE_STATUS_UNSUPPORTED;
    }

    *out_request = bridge->entries[bridge->head];
    bridge->head = (bridge->head + 1u) % VM_MACHINE_EXECUTOR_QUEUE_CAPACITY;
    --bridge->count;
    return TYPE_STATUS_OK;
}
