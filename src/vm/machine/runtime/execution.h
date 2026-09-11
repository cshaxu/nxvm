/* Machine-owned binding for the retained single-machine execution loop. */
#ifndef VM_MACHINE_EXECUTION_CONTEXT_H
#define VM_MACHINE_EXECUTION_CONTEXT_H


#include "type.h"

typedef struct vm_machine vm_machine;

typedef struct vm_machine_execution_context_callbacks {
    type_status (*reset)(vm_machine *machine);
    C_VOID (*debug_refresh)(vm_machine *machine);
} vm_machine_execution_context_callbacks;

typedef C_VOID (*vm_machine_execution_context_command_boundary)(C_VOID *opaque);

typedef struct vm_machine_execution_context {
    C_UINT generation;
    C_INT active;
    vm_machine *machine;
    vm_machine_execution_context_command_boundary command_boundary;
    C_VOID *command_boundary_opaque;
    const vm_machine_execution_context_callbacks *callbacks;
} vm_machine_execution_context;

C_VOID vm_machine_execution_context_initialize(vm_machine_execution_context *context);
C_VOID vm_machine_execution_context_activate(vm_machine_execution_context *context);
C_VOID vm_machine_execution_context_deactivate(vm_machine_execution_context *context);
C_VOID vm_machine_execution_context_bind_machine(
    vm_machine_execution_context *context, vm_machine *machine);
C_VOID vm_machine_execution_context_bind_callbacks(
    vm_machine_execution_context *context,
    const vm_machine_execution_context_callbacks *callbacks);
type_status vm_machine_execution_context_reset(vm_machine_execution_context *context);
C_VOID vm_machine_execution_context_debug_refresh(vm_machine_execution_context *context);
C_VOID vm_machine_execution_context_bind_command_boundary(
    vm_machine_execution_context *context,
    vm_machine_execution_context_command_boundary callback, C_VOID *opaque);
C_VOID vm_machine_execution_context_run_command_boundary(vm_machine_execution_context *context);

#endif
