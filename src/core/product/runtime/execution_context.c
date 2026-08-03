#include "core/product/runtime/execution_context.h"

void core_product_execution_context_initialize(core_product_execution_context *context)
{
    if (context != 0) {
        context->generation++;
        context->active = 0;
    }
}

void core_product_execution_context_activate(core_product_execution_context *context)
{
    if (context != 0) context->active = 1;
}

void core_product_execution_context_deactivate(core_product_execution_context *context)
{
    if (context != 0) context->active = 0;
}

void core_product_execution_context_bind_machine_state(
    core_product_execution_context *context, void *cpu, void *ram, void *port,
    void *device)
{
    if (context != 0) {
        context->cpu = cpu;
        context->ram = ram;
        context->port = port;
        context->device = device;
    }
}

void core_product_execution_context_bind_callbacks(
    core_product_execution_context *context,
    const core_product_execution_context_callbacks *callbacks)
{
    if (context != 0) {
        context->callbacks = callbacks;
    }
}

void *core_product_execution_context_cpu(const core_product_execution_context *context)
{
    return context != 0 ? context->cpu : 0;
}

void core_product_execution_context_reset(core_product_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->reset != 0) {
        context->callbacks->reset(context->device);
    }
}

void core_product_execution_context_debug_refresh(core_product_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->debug_refresh != 0) {
        context->callbacks->debug_refresh(context->device);
    }
}

void core_product_execution_context_bind_command_boundary(
    core_product_execution_context *context,
    core_product_execution_context_command_boundary callback, void *opaque)
{
    if (context != 0) {
        context->command_boundary = callback;
        context->command_boundary_opaque = opaque;
    }
}

void core_product_execution_context_run_command_boundary(core_product_execution_context *context)
{
    if (context != 0 && context->active &&
        context->command_boundary != 0) {
        context->command_boundary(context->command_boundary_opaque);
    }
}
