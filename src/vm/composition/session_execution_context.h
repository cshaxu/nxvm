/* VM-session-owned binding for the retained single-machine execution loop. */
#ifndef NTVDM64_VM_SESSION_EXECUTION_CONTEXT_H
#define NTVDM64_VM_SESSION_EXECUTION_CONTEXT_H


#include "type.h"

typedef struct vm_session vm_session;

typedef struct vm_session_execution_context_callbacks {
    C_VOID (*reset)(vm_session *session);
    C_VOID (*debug_refresh)(vm_session *session);
} vm_session_execution_context_callbacks;

typedef C_VOID (*vm_session_execution_context_command_boundary)(C_VOID *opaque);

typedef struct vm_session_execution_context {
    C_UINT generation;
    C_INT active;
    vm_session *session;
    vm_session_execution_context_command_boundary command_boundary;
    C_VOID *command_boundary_opaque;
    const vm_session_execution_context_callbacks *callbacks;
} vm_session_execution_context;

C_VOID vm_session_execution_context_initialize(vm_session_execution_context *context);
C_VOID vm_session_execution_context_activate(vm_session_execution_context *context);
C_VOID vm_session_execution_context_deactivate(vm_session_execution_context *context);
C_VOID vm_session_execution_context_bind_session(
    vm_session_execution_context *context, vm_session *session);
C_VOID vm_session_execution_context_bind_callbacks(
    vm_session_execution_context *context,
    const vm_session_execution_context_callbacks *callbacks);
C_VOID vm_session_execution_context_reset(vm_session_execution_context *context);
C_VOID vm_session_execution_context_debug_refresh(vm_session_execution_context *context);
C_VOID vm_session_execution_context_bind_command_boundary(
    vm_session_execution_context *context,
    vm_session_execution_context_command_boundary callback, C_VOID *opaque);
C_VOID vm_session_execution_context_run_command_boundary(vm_session_execution_context *context);

#endif
