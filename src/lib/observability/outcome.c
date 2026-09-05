#include "type.h"

#include "lib/observability/outcome.h"

struct lib_observability_outcome {
    lib_observability_outcome_snapshot snapshot;
};

type_status lib_observability_outcome_create(lib_observability_outcome **out_outcome)
{
    lib_observability_outcome *outcome;

    if (out_outcome == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_outcome = STD_NULL;
    outcome = STD_CALLOC(1u, sizeof(*outcome));
    if (outcome == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    outcome->snapshot.status = TYPE_STATUS_OK;
    *out_outcome = outcome;
    return TYPE_STATUS_OK;
}

C_VOID lib_observability_outcome_destroy(lib_observability_outcome *outcome)
{ STD_FREE(outcome); }

C_VOID lib_observability_outcome_clear(lib_observability_outcome *outcome)
{
    if (outcome == STD_NULL) return;
    outcome->snapshot.valid = TYPE_FALSE;
    outcome->snapshot.status = TYPE_STATUS_OK;
}

type_status lib_observability_outcome_publish(lib_observability_outcome *outcome,
    type_status status)
{
    if (outcome == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++outcome->snapshot.sequence;
    outcome->snapshot.status = status;
    outcome->snapshot.valid = TYPE_TRUE;
    return status;
}

type_status lib_observability_outcome_capture(
    const lib_observability_outcome *outcome,
    lib_observability_outcome_snapshot *out_snapshot)
{
    if (outcome == STD_NULL || out_snapshot == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_snapshot = outcome->snapshot;
    return TYPE_STATUS_OK;
}
