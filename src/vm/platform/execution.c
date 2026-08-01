#include "vm/platform/execution.h"

#include <string.h>

static vm_platform_execution_sink vmPlatformExecutionSink;
static void *vmPlatformExecutionContext;

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
