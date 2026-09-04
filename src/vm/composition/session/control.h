#ifndef VM_SESSION_CONTROL_H
#define VM_SESSION_CONTROL_H


#include "type.h"
#include "vm/composition/session/execution.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vm_session_pause_reason {
    VM_SESSION_PAUSE_NONE,
    VM_SESSION_PAUSE_EXPLICIT,
    VM_SESSION_PAUSE_BREAKPOINT,
    VM_SESSION_PAUSE_TRACE,
    VM_SESSION_PAUSE_STEP
} vm_session_pause_reason;

typedef struct vm_session vm_session;

typedef struct vm_session_control_state {
    STD_ATOMIC_BOOL flagFlip;
    STD_ATOMIC_BOOL flagRun;
    STD_ATOMIC_BOOL flagReset;
    STD_ATOMIC_BOOL pauseRequested;
    STD_ATOMIC_BOOL paused;
    STD_ATOMIC_BOOL stepRequested;
    STD_ATOMIC_INT pauseReason;
    vm_session_execution_context execution_context;
} vm_session_control_state;

#include "vm/composition/session/session_interface.h"

C_VOID vm_session_control_start(vm_session_control_state *control);
type_status vm_session_control_reset(vm_session_control_state *control);
C_VOID vm_session_control_stop(vm_session_control_state *control);
C_VOID vm_session_control_fault(vm_session_control_state *control);
C_VOID vm_session_control_request_pause(vm_session_control_state *control,
    vm_session_pause_reason reason);
C_INT vm_session_control_wait_for_pause(vm_session_control_state *control,
    C_UINT milliseconds);
C_INT vm_session_control_is_paused(const vm_session_control_state *control);
vm_session_pause_reason vm_session_control_get_pause_reason(
    const vm_session_control_state *control);
C_VOID vm_session_control_continue(vm_session_control_state *control);
C_INT vm_session_control_step(vm_session_control_state *control);
type_status vm_session_control_initialize(vm_session_control_state *control,
    vm_session *machine);
C_VOID vm_session_control_finalize(vm_session_control_state *control,
    vm_session *machine);
C_INT vm_session_control_is_running(const vm_session_control_state *control);
C_INT vm_session_control_get_flip(const vm_session_control_state *control);
C_VOID vm_session_control_print_status(const vm_session_control_state *control);
C_VOID vm_session_control_bind_command_boundary(
    vm_session_control_state *control,
    C_VOID (*callback)(C_VOID *opaque), C_VOID *opaque);

#ifdef __cplusplus
}
#endif

#endif
