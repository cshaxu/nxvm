#include "type.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/virtual_time.h"

type_status vm_session_virtual_time_on_waiting(vm_session *session,
    const core_machine_run_result *result, C_INT *out_advanced)
{
    type_unsigned_64 source_ticks = 0u;
    type_status status;

    if (session == STD_NULL || result == STD_NULL || out_advanced == STD_NULL ||
        session->core_machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_advanced = 0;
    if (result->reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        !vm_session_control_is_running(&session->control)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (session->speed == VM_SESSION_SPEED_TURBO) {
        source_ticks = 1u;
        status = TYPE_STATUS_OK;
    } else {
        if (session->virtual_time_source.next == STD_NULL) return TYPE_STATUS_OK;
        status = session->virtual_time_source.next(
            session->virtual_time_source.context, &source_ticks);
    }
    if (status != TYPE_STATUS_OK || source_ticks == 0u) return status;
    status = core_machine_advance_time(session->core_machine, source_ticks);
    if (status == TYPE_STATUS_OK) *out_advanced = 1;
    return status;
}

C_VOID vm_session_virtual_time_reset(vm_session *session)
{
    if (session != STD_NULL && session->virtual_time_source.reset != STD_NULL) {
        session->virtual_time_source.reset(session->virtual_time_source.context);
    }
}
