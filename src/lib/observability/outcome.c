#include "lib/base/base.h"

#include "lib/observability/outcome.h"

struct lib_observability_outcome {
    lib_observability_outcome_snapshot snapshot;
};

lib_status lib_observability_outcome_create(lib_observability_outcome **out_outcome)
{
    lib_observability_outcome *outcome;

    if (out_outcome == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_outcome = LIB_NULL;
    outcome = calloc(1u, sizeof(*outcome));
    if (outcome == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    outcome->snapshot.status = LIB_STATUS_OK;
    *out_outcome = outcome;
    return LIB_STATUS_OK;
}

void lib_observability_outcome_destroy(lib_observability_outcome *outcome)
{ free(outcome); }

void lib_observability_outcome_clear(lib_observability_outcome *outcome)
{
    if (outcome == LIB_NULL) return;
    outcome->snapshot.valid = LIB_FALSE;
    outcome->snapshot.status = LIB_STATUS_OK;
}

lib_status lib_observability_outcome_publish(lib_observability_outcome *outcome,
    lib_status status)
{
    if (outcome == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++outcome->snapshot.sequence;
    outcome->snapshot.status = status;
    outcome->snapshot.valid = LIB_TRUE;
    return status;
}

lib_status lib_observability_outcome_capture(
    const lib_observability_outcome *outcome,
    lib_observability_outcome_snapshot *out_snapshot)
{
    if (outcome == LIB_NULL || out_snapshot == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_snapshot = outcome->snapshot;
    return LIB_STATUS_OK;
}
