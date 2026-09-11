#ifndef VM_MACHINE_STATE_H
#define VM_MACHINE_STATE_H

#include "type.h"

typedef struct vm_machine_executor_state vm_machine_executor_state;

typedef enum vm_machine_lifecycle {
    VM_MACHINE_STOPPED,
    VM_MACHINE_RUNNING,
    VM_MACHINE_PAUSED,
    VM_MACHINE_RESET
} vm_machine_lifecycle;

type_status vm_machine_executor_state_create(vm_machine_executor_state **out_state);
void vm_machine_executor_state_destroy(vm_machine_executor_state *state);
void vm_machine_executor_state_start(vm_machine_executor_state *state);
void vm_machine_executor_state_stop(vm_machine_executor_state *state);
void vm_machine_executor_state_request_reset(vm_machine_executor_state *state);
int vm_machine_executor_state_take_reset(vm_machine_executor_state *state);
void vm_machine_executor_state_request_pause(vm_machine_executor_state *state);
void vm_machine_executor_state_acknowledge_pause(vm_machine_executor_state *state);
void vm_machine_executor_state_resume(vm_machine_executor_state *state);
int vm_machine_executor_state_is_active(const vm_machine_executor_state *state);
int vm_machine_executor_state_is_paused(const vm_machine_executor_state *state);
int vm_machine_executor_state_pause_requested(const vm_machine_executor_state *state);
vm_machine_lifecycle vm_machine_executor_state_lifecycle(const vm_machine_executor_state *state);

#endif
