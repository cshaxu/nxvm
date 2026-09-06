#ifndef LIB_OBSERVABILITY_OUTCOME_H
#define LIB_OBSERVABILITY_OUTCOME_H

#include "lib/base/base.h"

typedef struct lib_observability_outcome lib_observability_outcome;

typedef struct lib_observability_outcome_snapshot {
    lib_u64 sequence;
    lib_status status;
    int valid;
} lib_observability_outcome_snapshot;

/* Creation returns one owned outcome.  The owner externally serializes every
 * publish, capture, clear and destroy operation; a captured snapshot is a
 * copied value that remains valid after later outcome operations. */
lib_status lib_observability_outcome_create(lib_observability_outcome **out_outcome);
void lib_observability_outcome_destroy(lib_observability_outcome *outcome);
/* Invalidates the current result, preserves sequence, and restores OK status. */
void lib_observability_outcome_clear(lib_observability_outcome *outcome);
/* Publishes one status, increments sequence, marks the outcome valid, and
 * returns the same status for direct propagation. */
lib_status lib_observability_outcome_publish(lib_observability_outcome *outcome,
    lib_status status);
/* Captures the complete current copied result. */
lib_status lib_observability_outcome_capture(
    const lib_observability_outcome *outcome,
    lib_observability_outcome_snapshot *out_snapshot);

#endif
