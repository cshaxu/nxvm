#ifndef VM_MACHINE_WAITING_H
#define VM_MACHINE_WAITING_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_interface.h"

lib_status vm_machine_waiting_advance(vm_machine *session,
    const core_machine_run_result *result, lib_i32 *out_advanced);
lib_i32 vm_machine_pacing_ratio_compare(lib_u64 left_numerator,
    lib_u64 left_denominator, lib_u64 right_numerator,
    lib_u64 right_denominator);
lib_status vm_machine_pacing_wait(vm_machine *session);
void vm_machine_pacing_reset(vm_machine *session);

#endif
