#ifndef VM_SESSION_FAULT_H
#define VM_SESSION_FAULT_H

#include "type.h"

#include "core/machine/machine_interface.h"

typedef struct vm_session vm_session;

typedef struct vm_session_fault_outcome {
    C_INT valid;
    core_machine_run_result run;
    core_machine_cpu_diagnostic diagnostic;
} vm_session_fault_outcome;

C_VOID vm_session_fault_clear(vm_session *session);
C_VOID vm_session_fault_capture(vm_session *session,
    const core_machine_run_result *run);
C_INT vm_session_fault_get(const vm_session *session,
    vm_session_fault_outcome *out_outcome);
C_VOID vm_session_fault_print(const vm_session *session);

#endif
