#include "core/product/runtime/execution_context.h"

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

void nxvm_execution_context_bind_callbacks(
    nxvm_execution_context *context,
    const nxvm_execution_context_callbacks *callbacks)
{
    if (context != 0) {
        context->callbacks = callbacks;
    }
}

const nxvm_execution_context *nxvm_execution_context_current(void)
{
    return nxvm_current_execution_context;
}

void *nxvm_execution_context_cpu(const nxvm_execution_context *context)
{
    return context != 0 ? context->cpu : 0;
}

void nxvm_execution_context_reset(nxvm_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->reset != 0) {
        context->callbacks->reset(context->device);
    }
}

void nxvm_execution_context_debug_refresh(nxvm_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->debug_refresh != 0) {
        context->callbacks->debug_refresh(context->device);
    }
}

void nxvm_execution_context_machine_refresh(nxvm_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->machine_refresh != 0) {
        context->callbacks->machine_refresh(context->device);
    }
}

void nxvm_execution_context_bind_command_boundary(
    nxvm_execution_context *context,
    nxvm_execution_context_command_boundary callback, void *opaque)
{
    if (context != 0) {
        context->command_boundary = callback;
        context->command_boundary_opaque = opaque;
    }
}

void nxvm_execution_context_run_command_boundary(nxvm_execution_context *context)
{
    if (context != 0 && context->active &&
        context->command_boundary != 0) {
        context->command_boundary(context->command_boundary_opaque);
    }
}
