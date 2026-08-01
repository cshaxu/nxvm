#include "machine/vm/execution_context.h"

static nxvm_execution_context *nxvm_current_execution_context;

void nxvm_execution_context_initialize(nxvm_execution_context *context)
{
    if (context != 0) {
        context->generation++;
        context->active = 0;
    }
}

void nxvm_execution_context_enter(nxvm_execution_context *context)
{
    nxvm_current_execution_context = context;
    if (context != 0) context->active = 1;
}

void nxvm_execution_context_leave(nxvm_execution_context *context)
{
    if (nxvm_current_execution_context == context) nxvm_current_execution_context = 0;
    if (context != 0) context->active = 0;
}

void nxvm_execution_context_bind_machine_state(
    nxvm_execution_context *context, void *cpu, void *ram, void *port,
    void *device)
{
    if (context != 0) {
        context->cpu = cpu;
        context->ram = ram;
        context->port = port;
        context->device = device;
    }
}

const nxvm_execution_context *nxvm_execution_context_current(void)
{
    return nxvm_current_execution_context;
}
