#include "type.h"

#include "vm/composition/session/fault.h"
#include "vm/composition/session/session_private.h"

C_VOID vm_session_fault_clear(vm_session *session)
{
    if (session != STD_NULL) {
        STD_MEMSET(&session->fault_outcome, 0, sizeof(session->fault_outcome));
    }
}

C_VOID vm_session_fault_capture(vm_session *session,
    const core_machine_run_result *run)
{
    if (session == STD_NULL || run == STD_NULL) return;
    vm_session_fault_clear(session);
    session->fault_outcome.valid = TYPE_TRUE;
    session->fault_outcome.run = *run;
    (C_VOID)core_machine_get_cpu_diagnostic(session->core_machine,
        &session->fault_outcome.diagnostic);
}

C_INT vm_session_fault_get(const vm_session *session,
    vm_session_fault_outcome *out_outcome)
{
    if (session == STD_NULL || out_outcome == STD_NULL) return 1;
    *out_outcome = session->fault_outcome;
    return 0;
}

C_VOID vm_session_fault_print(const vm_session *session)
{
    const vm_session_fault_outcome *outcome;
    const core_machine_cpu_fault_snapshot *fault;

    if (session == STD_NULL || !session->fault_outcome.valid) return;
    outcome = &session->fault_outcome;
    STD_PRINTF("Fault:     detail=%08X pc=%08X\n", outcome->run.detail,
        outcome->run.linear_pc);
    fault = &outcome->diagnostic.first_fault;
    if (fault->valid) {
        STD_PRINTF("Exception: mask=%08X code=%08X at %04X:%08X\n",
            fault->exception_mask, fault->exception_code, fault->point.cs,
            fault->point.eip);
    }
}
