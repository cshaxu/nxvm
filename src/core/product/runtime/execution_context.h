/* Session-owned binding for the retained single-machine execution loop. */
#ifndef NXVM_MACHINE_VM_EXECUTION_CONTEXT_H
#define NXVM_MACHINE_VM_EXECUTION_CONTEXT_H


#include "type.h"
typedef struct core_product_execution_context_callbacks {
    C_VOID (*reset)(C_VOID *device);
    C_VOID (*debug_refresh)(C_VOID *device);
} core_product_execution_context_callbacks;

typedef C_VOID (*core_product_execution_context_command_boundary)(C_VOID *opaque);

typedef struct core_product_execution_context {
    C_UINT generation;
    C_INT active;
    C_VOID *cpu;
    C_VOID *ram;
    C_VOID *port;
    C_VOID *device;
    core_product_execution_context_command_boundary command_boundary;
    C_VOID *command_boundary_opaque;
    const core_product_execution_context_callbacks *callbacks;
} core_product_execution_context;

C_VOID core_product_execution_context_initialize(core_product_execution_context *context);
C_VOID core_product_execution_context_activate(core_product_execution_context *context);
C_VOID core_product_execution_context_deactivate(core_product_execution_context *context);
C_VOID core_product_execution_context_bind_machine_state(
    core_product_execution_context *context, C_VOID *cpu, C_VOID *ram, C_VOID *port,
    C_VOID *device);
C_VOID core_product_execution_context_bind_callbacks(
    core_product_execution_context *context,
    const core_product_execution_context_callbacks *callbacks);
C_VOID *core_product_execution_context_cpu(const core_product_execution_context *context);
C_VOID core_product_execution_context_reset(core_product_execution_context *context);
C_VOID core_product_execution_context_debug_refresh(core_product_execution_context *context);
C_VOID core_product_execution_context_bind_command_boundary(
    core_product_execution_context *context,
    core_product_execution_context_command_boundary callback, C_VOID *opaque);
C_VOID core_product_execution_context_run_command_boundary(core_product_execution_context *context);

#endif
