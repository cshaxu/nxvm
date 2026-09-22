#ifndef VM_MACHINE_CONTROL_H
#define VM_MACHINE_CONTROL_H

#include "type.h"
#include "core/machine/executor_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vm_machine vm_machine;

typedef struct vm_machine_control_state {
    vm_machine_executor_state *state;
    vm_machine *machine;
} vm_machine_control_state;

#include "core/machine/machine_interface.h"

C_VOID vm_machine_control_start(vm_machine_control_state *control);
type_status vm_machine_control_reset(vm_machine_control_state *control);
C_VOID vm_machine_control_stop(vm_machine_control_state *control);
C_VOID vm_machine_control_fault(vm_machine_control_state *control);
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
