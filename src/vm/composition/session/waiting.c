#include "type.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"

type_status vm_session_waiting_advance(vm_session *session,
    const core_machine_run_result *result, C_INT *out_advanced)
{
    core_machine_pacing_contract contract;
    type_bool advanced;
    type_status status;

    if (session == STD_NULL || result == STD_NULL || out_advanced == STD_NULL ||
        session->core_machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_advanced = 0;
    if (result->reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        !vm_session_control_is_running(&session->control)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    status = core_machine_get_pacing_contract(session->core_machine, &contract);
    if (status != TYPE_STATUS_OK || !contract.available) return status;
    status = core_machine_advance_to_next_deadline(session->core_machine, &advanced);
    if (status == TYPE_STATUS_OK && advanced) *out_advanced = 1;
    return status;
}
