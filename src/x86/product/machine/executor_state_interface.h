#ifndef VM_MACHINE_STATE_H
#define VM_MACHINE_STATE_H
#include "lib/types/types_interface.h"


typedef struct vm_machine_executor_state vm_machine_executor_state;

lib_status vm_machine_executor_state_create(vm_machine_executor_state **out_state);
void vm_machine_executor_state_destroy(vm_machine_executor_state *state);
void vm_machine_executor_state_start(vm_machine_executor_state *state);
void vm_machine_executor_state_stop(vm_machine_executor_state *state);
void vm_machine_executor_state_request_reset(vm_machine_executor_state *state);
lib_i32 vm_machine_executor_state_take_reset(vm_machine_executor_state *state);
lib_i32 vm_machine_executor_state_is_active(const vm_machine_executor_state *state);

#endif
