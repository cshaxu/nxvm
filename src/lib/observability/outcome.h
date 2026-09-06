#ifndef LIB_OBSERVABILITY_OUTCOME_H
#define LIB_OBSERVABILITY_OUTCOME_H

#include "lib/base/base.h"

typedef struct lib_observability_outcome lib_observability_outcome;

typedef struct lib_observability_outcome_snapshot {
    lib_u64 sequence;
    lib_status status;
    int valid;
} lib_observability_outcome_snapshot;

lib_status lib_observability_outcome_create(lib_observability_outcome **out_outcome);
void lib_observability_outcome_destroy(lib_observability_outcome *outcome);
void lib_observability_outcome_clear(lib_observability_outcome *outcome);
lib_status lib_observability_outcome_publish(lib_observability_outcome *outcome,
    lib_status status);
lib_status lib_observability_outcome_capture(
    const lib_observability_outcome *outcome,
    lib_observability_outcome_snapshot *out_snapshot);

#endif
