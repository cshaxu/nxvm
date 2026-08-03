#include "type.h"

#include "vm/composition/session_execution_context.h"

C_VOID vm_session_execution_context_initialize(vm_session_execution_context *context)
{
    if (context != 0) {
        context->generation++;
        context->active = 0;
    }
}

C_VOID vm_session_execution_context_activate(vm_session_execution_context *context)
{
    if (context != 0) context->active = 1;
}

C_VOID vm_session_execution_context_deactivate(vm_session_execution_context *context)
{
    if (context != 0) context->active = 0;
}

C_VOID vm_session_execution_context_bind_machine_state(
    vm_session_execution_context *context, C_VOID *cpu, C_VOID *ram, C_VOID *port,
    C_VOID *device)
{
    if (context != 0) {
        context->cpu = cpu;
        context->ram = ram;
        context->port = port;
        context->device = device;
    }
}

C_VOID vm_session_execution_context_bind_callbacks(
    vm_session_execution_context *context,
    const vm_session_execution_context_callbacks *callbacks)
{
    if (context != 0) {
        context->callbacks = callbacks;
    }
}

C_VOID *vm_session_execution_context_cpu(const vm_session_execution_context *context)
{
    return context != 0 ? context->cpu : 0;
}

C_VOID vm_session_execution_context_reset(vm_session_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->reset != 0) {
        context->callbacks->reset(context->device);
    }
}

C_VOID vm_session_execution_context_debug_refresh(vm_session_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->debug_refresh != 0) {
        context->callbacks->debug_refresh(context->device);
    }
}

C_VOID vm_session_execution_context_bind_command_boundary(
    vm_session_execution_context *context,
    vm_session_execution_context_command_boundary callback, C_VOID *opaque)
{
    if (context != 0) {
        context->command_boundary = callback;
        context->command_boundary_opaque = opaque;
    }
}

C_VOID vm_session_execution_context_run_command_boundary(vm_session_execution_context *context)
{
    if (context != 0 && context->active &&
        context->command_boundary != 0) {
        context->command_boundary(context->command_boundary_opaque);
    }
}
