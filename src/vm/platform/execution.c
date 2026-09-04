#include "type.h"

#include "core/platform/wait_interface.h"
#include "vm/platform/execution.h"



struct vm_platform_execution_transport {
    const vm_platform_execution_sink *sink;
    C_VOID *context;
};

type_status vm_platform_execution_transport_create(
    const vm_platform_execution_sink *sink, C_VOID *context,
    vm_platform_execution_transport **out_transport)
{
    vm_platform_execution_transport *transport;

    if (out_transport == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_transport = STD_NULL;
    transport = STD_MALLOC(sizeof(*transport));
    if (transport == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    transport->sink = sink;
    transport->context = context;
    *out_transport = transport;
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_execution_transport_destroy(
    vm_platform_execution_transport *transport)
{
    STD_FREE(transport);
}

C_INT vm_platform_execution_is_running_for(
    const vm_platform_execution_transport *transport)
{
    return vm_platform_execution_get_lifecycle_for(transport) ==
        VM_PLATFORM_EXECUTION_RUNNING;
}

vm_platform_execution_lifecycle vm_platform_execution_get_lifecycle_for(
    const vm_platform_execution_transport *transport)
{
    return transport == STD_NULL || transport->sink == STD_NULL ||
        transport->sink->get_lifecycle == STD_NULL ?
        VM_PLATFORM_EXECUTION_STOPPED :
        transport->sink->get_lifecycle(transport->context);
}

C_INT vm_platform_execution_get_flip_for(
    const vm_platform_execution_transport *transport)
{
    return transport == STD_NULL || transport->sink == STD_NULL ||
        transport->sink->get_flip == STD_NULL ? 0 :
        transport->sink->get_flip(transport->context);
}

C_VOID vm_platform_execution_start_for(
    const vm_platform_execution_transport *transport)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->start != STD_NULL) {
        transport->sink->start(transport->context);
    }
}

C_VOID vm_platform_execution_stop_for(
    const vm_platform_execution_transport *transport)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->stop != STD_NULL) {
        transport->sink->stop(transport->context);
    }
}

typedef struct vm_platform_execution_flip_wait {
    const vm_platform_execution_transport *transport;
    C_INT initial_flip;
} vm_platform_execution_flip_wait;

static C_INT vm_platform_execution_flip_wait_cancelled(C_VOID *context)
{
    const vm_platform_execution_flip_wait *wait = context;

    return wait == STD_NULL || vm_platform_execution_get_flip_for(
        wait->transport) != wait->initial_flip;
}

C_INT vm_platform_execution_wait_for_flip_for(
    const vm_platform_execution_transport *transport, C_INT initial_flip,
    type_unsigned_32 timeout_milliseconds)
{
    vm_platform_execution_flip_wait wait = { transport, initial_flip };

    if (transport == STD_NULL) return TYPE_FALSE;
    (C_VOID)core_platform_wait_milliseconds(timeout_milliseconds,
        vm_platform_execution_flip_wait_cancelled, &wait);
    return vm_platform_execution_get_flip_for(transport) != initial_flip;
}
