#ifndef NTVDM64_VM_COMPOSITION_CONTROL_H
#define NTVDM64_VM_COMPOSITION_CONTROL_H

#include "type.h"
#include "vm/composition_live_machine.h"

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

void vm_composition_control_start(void);
void vm_composition_control_reset(void);
void vm_composition_control_stop(void);
void vm_composition_control_request_pause(vm_composition_pause_reason reason);
int vm_composition_control_wait_for_pause(unsigned milliseconds);
int vm_composition_control_is_paused(void);
vm_composition_pause_reason vm_composition_control_get_pause_reason(void);
void vm_composition_control_continue(void);
int vm_composition_control_step(void);
void vm_composition_control_initialize(vm_composition_live_machine *machine);
void vm_composition_control_finalize(vm_composition_live_machine *machine);
int vm_composition_control_is_running(void);
int vm_composition_control_get_flip(void);
void vm_composition_control_print_status(void);
void vm_composition_control_bind_command_boundary(
    void (*callback)(void *opaque), void *opaque);

#ifdef __cplusplus
}
#endif

#endif
