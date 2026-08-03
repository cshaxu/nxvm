/* Session-owned binding for the retained single-machine execution loop. */
#ifndef NXVM_MACHINE_VM_EXECUTION_CONTEXT_H
#define NXVM_MACHINE_VM_EXECUTION_CONTEXT_H

typedef struct nxvm_execution_context_callbacks {
    void (*reset)(void *device);
    void (*debug_refresh)(void *device);
} nxvm_execution_context_callbacks;

typedef void (*nxvm_execution_context_command_boundary)(void *opaque);

typedef struct nxvm_execution_context {
    unsigned generation;
    int active;
    void *cpu;
    void *ram;
    void *port;
    void *device;
    nxvm_execution_context_command_boundary command_boundary;
    void *command_boundary_opaque;
    const nxvm_execution_context_callbacks *callbacks;
} nxvm_execution_context;

void nxvm_execution_context_initialize(nxvm_execution_context *context);
void nxvm_execution_context_activate(nxvm_execution_context *context);
void nxvm_execution_context_deactivate(nxvm_execution_context *context);
void nxvm_execution_context_bind_machine_state(
    nxvm_execution_context *context, void *cpu, void *ram, void *port,
    void *device);
void nxvm_execution_context_bind_callbacks(
    nxvm_execution_context *context,
    const nxvm_execution_context_callbacks *callbacks);
void *nxvm_execution_context_cpu(const nxvm_execution_context *context);
void nxvm_execution_context_reset(nxvm_execution_context *context);
void nxvm_execution_context_debug_refresh(nxvm_execution_context *context);
void nxvm_execution_context_bind_command_boundary(
    nxvm_execution_context *context,
    nxvm_execution_context_command_boundary callback, void *opaque);
void nxvm_execution_context_run_command_boundary(nxvm_execution_context *context);

#endif
