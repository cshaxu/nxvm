#include "type.h"

#include "core/utils/wait.h"
#include "vm/platform/execution.h"



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

C_INT vm_platform_execution_wait_for_flip_for(
    const vm_platform_execution_transport *transport, C_INT initial_flip,
    const core_utils_wait_scope *wait_scope, uint32_t timeout_milliseconds)
{
    uint32_t waited;

    for (waited = 0u; waited < timeout_milliseconds; ) {
        uint32_t interval = timeout_milliseconds - waited;

        if (vm_platform_execution_get_flip_for(transport) != initial_flip) {
            return TYPE_TRUE;
        }
        if (interval > 100u) interval = 100u;
        core_utils_wait_milliseconds(wait_scope, interval);
        waited += interval;
    }
    return vm_platform_execution_get_flip_for(transport) != initial_flip;
}
