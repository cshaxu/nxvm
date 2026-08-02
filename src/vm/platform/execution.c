#include "vm/platform/execution.h"

#include <string.h>

static vm_platform_execution_sink vmPlatformExecutionSink;
static void *vmPlatformExecutionContext;

void vm_platform_execution_transport_initialize(
    vm_platform_execution_transport *transport,
    const vm_platform_execution_sink *sink, void *context)
{
    if (transport == NULL) return;
    transport->sink = sink;
    transport->context = context;
}

int vm_platform_execution_is_running_for(
    const vm_platform_execution_transport *transport)
{
    return transport == NULL || transport->sink == NULL ||
        transport->sink->is_running == NULL ? 0 :
        transport->sink->is_running(transport->context);
}

int vm_platform_execution_get_flip_for(
    const vm_platform_execution_transport *transport)
{
    return transport == NULL || transport->sink == NULL ||
        transport->sink->get_flip == NULL ? 0 :
        transport->sink->get_flip(transport->context);
}

void vm_platform_execution_start_for(
    const vm_platform_execution_transport *transport)
{
    if (transport != NULL && transport->sink != NULL &&
        transport->sink->start != NULL) {
        transport->sink->start(transport->context);
    }
}

void vm_platform_execution_stop_for(
    const vm_platform_execution_transport *transport)
{
    if (transport != NULL && transport->sink != NULL &&
        transport->sink->stop != NULL) {
        transport->sink->stop(transport->context);
    }
}

void vm_platform_execution_bind(const vm_platform_execution_sink *sink,
                                void *context)
{
    if (sink == NULL) {
        memset(&vmPlatformExecutionSink, 0, sizeof(vmPlatformExecutionSink));
        vmPlatformExecutionContext = NULL;
        return;
    }
    vmPlatformExecutionSink = *sink;
    vmPlatformExecutionContext = context;
}

int vm_platform_execution_is_running(void)
{
    return vmPlatformExecutionSink.is_running == NULL ? 0 :
        vmPlatformExecutionSink.is_running(vmPlatformExecutionContext);
}

int vm_platform_execution_get_flip(void)
{
    return vmPlatformExecutionSink.get_flip == NULL ? 0 :
        vmPlatformExecutionSink.get_flip(vmPlatformExecutionContext);
}

void vm_platform_execution_start(void)
{
    if (vmPlatformExecutionSink.start != NULL) {
        vmPlatformExecutionSink.start(vmPlatformExecutionContext);
    }
}

void vm_platform_execution_stop(void)
{
    if (vmPlatformExecutionSink.stop != NULL) {
        vmPlatformExecutionSink.stop(vmPlatformExecutionContext);
    }
}
