#ifndef VM_MACHINE_FAULT_H
#define VM_MACHINE_FAULT_H

#include "type.h"

#include "core/machine/machine_interface.h"

typedef struct vm_machine vm_machine;

typedef struct vm_machine_fault_outcome {
    C_INT valid;
    core_machine_run_result run;
    core_machine_cpu_diagnostic diagnostic;
} vm_machine_fault_outcome;

C_VOID vm_machine_fault_clear(vm_machine *session);
C_VOID vm_machine_fault_capture(vm_machine *session,
    const core_machine_run_result *run);
C_INT vm_machine_fault_get(const vm_machine *session,
    vm_machine_fault_outcome *out_outcome);
C_VOID vm_machine_fault_print(const vm_machine *session);

#endif
