#ifndef VM_MACHINE_WAITING_H
#define VM_MACHINE_WAITING_H

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/machine/runtime/machine_interface.h"

type_status vm_machine_waiting_advance(vm_machine *session,
    const core_machine_run_result *result, C_INT *out_advanced);
C_INT vm_machine_pacing_ratio_compare(type_unsigned_64 left_numerator,
    type_unsigned_64 left_denominator, type_unsigned_64 right_numerator,
    type_unsigned_64 right_denominator);
type_status vm_machine_pacing_wait(vm_machine *session);
C_VOID vm_machine_pacing_reset(vm_machine *session);

#endif
