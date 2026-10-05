#ifndef VM_MACHINE_CONTROL_H
#define VM_MACHINE_CONTROL_H
#include "lib/types/types_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vm_machine vm_machine;

typedef struct vm_machine_executor_state {
    lib_atomic_i32 active;
    lib_atomic_i32 reset_requested;
} vm_machine_executor_state;

typedef struct vm_machine_control_state {
    vm_machine_executor_state state;
    vm_machine *machine;
} vm_machine_control_state;

#include "ibmpc/machine/machine_interface.h"

void vm_machine_executor_state_start(vm_machine_executor_state *state);
void vm_machine_executor_state_stop(vm_machine_executor_state *state);
void vm_machine_executor_state_request_reset(vm_machine_executor_state *state);
lib_bool vm_machine_executor_state_take_reset(vm_machine_executor_state *state);
lib_bool vm_machine_executor_state_is_active(const vm_machine_executor_state *state);

void vm_machine_control_start(vm_machine_control_state *control);
lib_status vm_machine_control_reset(vm_machine_control_state *control);
void vm_machine_control_stop(vm_machine_control_state *control);
void vm_machine_control_fault(vm_machine_control_state *control);
lib_status vm_machine_control_reset_at_boundary(vm_machine_control_state *control);
void vm_machine_control_refresh_debug(vm_machine_control_state *control);
lib_status vm_machine_control_initialize(vm_machine_control_state *control,
    vm_machine *machine);
void vm_machine_control_finalize(vm_machine_control_state *control,
    vm_machine *machine);
lib_bool vm_machine_control_is_running(const vm_machine_control_state *control);

#ifdef __cplusplus
}
#endif

#endif
