/* Session-owned binding for the retained single-machine execution loop. */
#ifndef NXVM_MACHINE_VM_EXECUTION_CONTEXT_H
#define NXVM_MACHINE_VM_EXECUTION_CONTEXT_H

typedef struct core_product_execution_context_callbacks {
    void (*reset)(void *device);
    void (*debug_refresh)(void *device);
} core_product_execution_context_callbacks;

typedef void (*core_product_execution_context_command_boundary)(void *opaque);

typedef struct core_product_execution_context {
    unsigned generation;
    int active;
    void *cpu;
    void *ram;
    void *port;
    void *device;
    core_product_execution_context_command_boundary command_boundary;
    void *command_boundary_opaque;
    const core_product_execution_context_callbacks *callbacks;
} core_product_execution_context;

void core_product_execution_context_initialize(core_product_execution_context *context);
void core_product_execution_context_activate(core_product_execution_context *context);
void core_product_execution_context_deactivate(core_product_execution_context *context);
void core_product_execution_context_bind_machine_state(
    core_product_execution_context *context, void *cpu, void *ram, void *port,
    void *device);
void core_product_execution_context_bind_callbacks(
    core_product_execution_context *context,
    const core_product_execution_context_callbacks *callbacks);
void *core_product_execution_context_cpu(const core_product_execution_context *context);
void core_product_execution_context_reset(core_product_execution_context *context);
void core_product_execution_context_debug_refresh(core_product_execution_context *context);
void core_product_execution_context_bind_command_boundary(
    core_product_execution_context *context,
    core_product_execution_context_command_boundary callback, void *opaque);
void core_product_execution_context_run_command_boundary(core_product_execution_context *context);

#endif
