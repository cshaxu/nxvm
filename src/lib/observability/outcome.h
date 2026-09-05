#ifndef LIB_OBSERVABILITY_OUTCOME_H
#define LIB_OBSERVABILITY_OUTCOME_H

#include "type.h"

typedef struct lib_observability_outcome lib_observability_outcome;

typedef struct lib_observability_outcome_snapshot {
    type_unsigned_64 sequence;
    type_status status;
    C_INT valid;
} lib_observability_outcome_snapshot;

type_status lib_observability_outcome_create(lib_observability_outcome **out_outcome);
C_VOID lib_observability_outcome_destroy(lib_observability_outcome *outcome);
C_VOID lib_observability_outcome_clear(lib_observability_outcome *outcome);
type_status lib_observability_outcome_publish(lib_observability_outcome *outcome,
    type_status status);
type_status lib_observability_outcome_capture(
    const lib_observability_outcome *outcome,
    lib_observability_outcome_snapshot *out_snapshot);

#endif
