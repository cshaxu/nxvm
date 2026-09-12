#include "type.h"

#include "vm/machine/runtime/fault.h"
#include "vm/machine/runtime/machine_private.h"

C_VOID vm_machine_fault_clear(vm_machine *session)
{
    if (session != STD_NULL) {
        STD_MEMSET(&session->fault_outcome, 0, sizeof(session->fault_outcome));
    }
}

C_VOID vm_machine_fault_capture(vm_machine *session,
    const core_machine_run_result *run)
{
    vm_machine_result result = {0};

    if (session == STD_NULL || run == STD_NULL) return;
    vm_machine_fault_clear(session);
    session->fault_outcome.valid = TYPE_TRUE;
    session->fault_outcome.run = *run;
    (C_VOID)core_machine_get_cpu_diagnostic(session->core_machine,
        &session->fault_outcome.diagnostic);
    result.kind = VM_MACHINE_RESULT_FAULT;
    result.status = TYPE_STATUS_FAULT;
    vm_machine_publish_result(session, &result);
}

C_INT vm_machine_fault_get(const vm_machine *session,
    vm_machine_fault_outcome *out_outcome)
{
    if (session == STD_NULL || out_outcome == STD_NULL) return 1;
    *out_outcome = session->fault_outcome;
    return 0;
}
