#ifndef NTVDM64_VM_COMPOSITION_CONTROL_H
#define NTVDM64_VM_COMPOSITION_CONTROL_H


#include "type.h"
#include "core/product/runtime/execution_context.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vm_composition_pause_reason {
    VM_COMPOSITION_PAUSE_NONE,
    VM_COMPOSITION_PAUSE_EXPLICIT,
    VM_COMPOSITION_PAUSE_BREAKPOINT,
    VM_COMPOSITION_PAUSE_TRACE,
    VM_COMPOSITION_PAUSE_STEP
} vm_composition_pause_reason;

typedef struct vm_composition_live_machine vm_composition_live_machine;

typedef struct vm_composition_control_state {
    STD_ATOMIC_BOOL flagFlip;
    STD_ATOMIC_BOOL flagRun;
    STD_ATOMIC_BOOL flagReset;
    STD_ATOMIC_BOOL pauseRequested;
    STD_ATOMIC_BOOL paused;
    STD_ATOMIC_BOOL stepRequested;
    STD_ATOMIC_INT pauseReason;
    core_product_execution_context execution_context;
} vm_composition_control_state;

#include "vm/composition/composition_live_machine.h"

C_VOID vm_composition_control_start(vm_composition_control_state *control);
C_VOID vm_composition_control_reset(vm_composition_control_state *control);
C_VOID vm_composition_control_stop(vm_composition_control_state *control);
C_VOID vm_composition_control_request_pause(vm_composition_control_state *control,
    vm_composition_pause_reason reason);
C_INT vm_composition_control_wait_for_pause(vm_composition_control_state *control,
    C_UINT milliseconds);
C_INT vm_composition_control_is_paused(const vm_composition_control_state *control);
vm_composition_pause_reason vm_composition_control_get_pause_reason(
    const vm_composition_control_state *control);
C_VOID vm_composition_control_continue(vm_composition_control_state *control);
C_INT vm_composition_control_step(vm_composition_control_state *control);
C_VOID vm_composition_control_initialize(vm_composition_control_state *control,
    vm_composition_live_machine *machine);
C_VOID vm_composition_control_finalize(vm_composition_control_state *control,
    vm_composition_live_machine *machine);
C_INT vm_composition_control_is_running(const vm_composition_control_state *control);
C_INT vm_composition_control_get_flip(const vm_composition_control_state *control);
C_VOID vm_composition_control_print_status(const vm_composition_control_state *control);
C_VOID vm_composition_control_bind_command_boundary(
    vm_composition_control_state *control,
    C_VOID (*callback)(C_VOID *opaque), C_VOID *opaque);

#ifdef __cplusplus
}
#endif

#endif
