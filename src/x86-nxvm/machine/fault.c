#include "lib/types/types_interface.h"

#include "x86-nxvm/machine/fault.h"
#include "x86-nxvm/machine/machine_private.h"

void x86_nxvm_machine_fault_clear(x86_nxvm_machine *session)
{
    if (session != LIB_NULL) {
        lib_memory_set(&session->fault_outcome, 0, sizeof(session->fault_outcome));
    }
}

void x86_nxvm_machine_fault_capture(x86_nxvm_machine *session,
    const x86_core_machine_run_result *run)
{
    if (session == LIB_NULL || run == LIB_NULL) return;
    x86_nxvm_machine_fault_clear(session);
    session->fault_outcome.valid = LIB_TRUE;
    session->fault_outcome.run = *run;
    (void)x86_core_machine_get_cpu_diagnostic(session->x86_core_machine,
        &session->fault_outcome.diagnostic);
}

lib_i32 x86_nxvm_machine_fault_get(const x86_nxvm_machine *session,
    x86_nxvm_machine_fault_outcome *out_outcome)
{
    if (session == LIB_NULL || out_outcome == LIB_NULL) return 1;
    *out_outcome = session->fault_outcome;
    return 0;
}
