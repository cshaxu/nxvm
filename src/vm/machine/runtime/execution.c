#include "type.h"

#include "vm/machine/runtime/machine_private.h"

#include "vm/machine/runtime/execution.h"

C_VOID vm_machine_execution_context_initialize(vm_machine_execution_context *context)
{
    if (context != 0) {
        context->generation++;
        context->active = 0;
    }
}

C_VOID vm_machine_execution_context_activate(vm_machine_execution_context *context)
{
    if (context != 0) context->active = 1;
}

C_VOID vm_machine_execution_context_deactivate(vm_machine_execution_context *context)
{
    if (context != 0) context->active = 0;
}

C_VOID vm_machine_execution_context_bind_machine(
    vm_machine_execution_context *context, vm_machine *machine)
{
    if (context != 0) {
        context->machine = machine;
    }
}

C_VOID vm_machine_execution_context_bind_callbacks(
    vm_machine_execution_context *context,
    const vm_machine_execution_context_callbacks *callbacks)
{
    if (context != 0) {
        context->callbacks = callbacks;
    }
}

type_status vm_machine_execution_context_reset(vm_machine_execution_context *context)
{
    if (context == 0 || context->callbacks == 0 ||
        context->callbacks->reset == 0) return TYPE_STATUS_INVALID_STATE;
    return context->callbacks->reset(context->machine);
}

C_VOID vm_machine_execution_context_debug_refresh(vm_machine_execution_context *context)
{
    if (context != 0 && context->callbacks != 0 &&
        context->callbacks->debug_refresh != 0) {
        context->callbacks->debug_refresh(context->machine);
    }
}

C_VOID vm_machine_execution_context_bind_command_boundary(
    vm_machine_execution_context *context,
    vm_machine_execution_context_command_boundary callback, C_VOID *opaque)
{
    if (context != 0) {
        context->command_boundary = callback;
        context->command_boundary_opaque = opaque;
    }
}

C_VOID vm_machine_execution_context_run_command_boundary(vm_machine_execution_context *context)
{
    if (context != 0 && context->active &&
        context->command_boundary != 0) {
        context->command_boundary(context->command_boundary_opaque);
    }
}
