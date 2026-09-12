#ifndef VM_MACHINE_CONTROL_H
#define VM_MACHINE_CONTROL_H

#include "type.h"
#include "lib/host/sync_interface.h"
#include "vm/machine/runtime/executor_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vm_machine_pause_reason {
    VM_MACHINE_PAUSE_NONE,
    VM_MACHINE_PAUSE_EXPLICIT,
    VM_MACHINE_PAUSE_BREAKPOINT,
    VM_MACHINE_PAUSE_TRACE,
    VM_MACHINE_PAUSE_STEP
} vm_machine_pause_reason;

typedef struct vm_machine vm_machine;

typedef struct vm_machine_control_state {
    vm_machine_executor_state *state;
    vm_machine *machine;
    atomic_bool step_requested;
    atomic_int pause_reason;
    host_sync_event *completion_ready;
    host_sync_event *control_changed;
} vm_machine_control_state;

#include "vm/machine/runtime/machine_interface.h"

C_VOID vm_machine_control_start(vm_machine_control_state *control);
type_status vm_machine_control_reset(vm_machine_control_state *control);
C_VOID vm_machine_control_stop(vm_machine_control_state *control);
C_VOID vm_machine_control_fault(vm_machine_control_state *control);
C_VOID vm_machine_control_request_pause(vm_machine_control_state *control,
    vm_machine_pause_reason reason);
C_INT vm_machine_control_wait_for_pause(vm_machine_control_state *control,
    C_UINT milliseconds);
C_INT vm_machine_control_wait_for_completion(
    vm_machine_control_state *control);
C_VOID vm_machine_control_signal_completion(
    vm_machine_control_state *control);
C_INT vm_machine_control_is_paused(const vm_machine_control_state *control);
vm_machine_pause_reason vm_machine_control_get_pause_reason(
    const vm_machine_control_state *control);
C_VOID vm_machine_control_continue(vm_machine_control_state *control);
C_INT vm_machine_control_step(vm_machine_control_state *control);
C_INT vm_machine_control_step_requested(const vm_machine_control_state *control);
C_INT vm_machine_control_take_step(vm_machine_control_state *control);
type_status vm_machine_control_reset_at_boundary(vm_machine_control_state *control);
C_VOID vm_machine_control_refresh_debug(vm_machine_control_state *control);
type_status vm_machine_control_initialize(vm_machine_control_state *control,
    vm_machine *machine);
C_VOID vm_machine_control_finalize(vm_machine_control_state *control,
    vm_machine *machine);
C_INT vm_machine_control_is_running(const vm_machine_control_state *control);

#ifdef __cplusplus
}
#endif

#endif
