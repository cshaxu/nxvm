#include "type.h"

#include "vm/platform/execution.h"


#include <stddef.h>

C_VOID vm_platform_execution_transport_initialize(
    vm_platform_execution_transport *transport,
    const vm_platform_execution_sink *sink, C_VOID *context)
{
    if (transport == STD_NULL) return;
    transport->sink = sink;
    transport->context = context;
}

C_INT vm_platform_execution_is_running_for(
    const vm_platform_execution_transport *transport)
{
    return transport == STD_NULL || transport->sink == STD_NULL ||
        transport->sink->is_running == STD_NULL ? 0 :
        transport->sink->is_running(transport->context);
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
