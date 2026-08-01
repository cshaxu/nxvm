/* Session-owned binding for the retained single-machine execution loop. */
#ifndef NXVM_MACHINE_VM_EXECUTION_CONTEXT_H
#define NXVM_MACHINE_VM_EXECUTION_CONTEXT_H

typedef struct nxvm_execution_context {
    unsigned generation;
    int active;
} nxvm_execution_context;

void nxvm_execution_context_initialize(nxvm_execution_context *context);
void nxvm_execution_context_enter(nxvm_execution_context *context);
void nxvm_execution_context_leave(nxvm_execution_context *context);
const nxvm_execution_context *nxvm_execution_context_current(void);

#endif
