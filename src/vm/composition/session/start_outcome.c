#include "type.h"

#include "vm/composition/session/start_outcome.h"

struct vm_session_start_outcome {
    vm_session_start_outcome_snapshot snapshot;
};

type_status vm_session_start_outcome_create(vm_session_start_outcome **out_outcome)
{
    vm_session_start_outcome *outcome;

    if (out_outcome == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_outcome = STD_NULL;
    outcome = calloc(1u, sizeof(*outcome));
    if (outcome == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    outcome->snapshot.status = TYPE_STATUS_OK;
    *out_outcome = outcome;
    return TYPE_STATUS_OK;
}

void vm_session_start_outcome_destroy(vm_session_start_outcome *outcome)
{ free(outcome); }

void vm_session_start_outcome_clear(vm_session_start_outcome *outcome)
{
    if (outcome == STD_NULL) return;
    outcome->snapshot.valid = TYPE_FALSE;
    outcome->snapshot.status = TYPE_STATUS_OK;
}

type_status vm_session_start_outcome_publish(vm_session_start_outcome *outcome,
    type_status status)
{
    if (outcome == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++outcome->snapshot.sequence;
    outcome->snapshot.status = status;
    outcome->snapshot.valid = TYPE_TRUE;
    return status;
}

type_status vm_session_start_outcome_capture(
    const vm_session_start_outcome *outcome,
    vm_session_start_outcome_snapshot *out_snapshot)
{
    if (outcome == STD_NULL || out_snapshot == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_snapshot = outcome->snapshot;
    return TYPE_STATUS_OK;
}
