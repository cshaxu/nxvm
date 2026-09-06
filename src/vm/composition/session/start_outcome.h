#ifndef VM_SESSION_START_OUTCOME_H
#define VM_SESSION_START_OUTCOME_H

#include "type.h"

typedef struct vm_session_start_outcome vm_session_start_outcome;

typedef struct vm_session_start_outcome_snapshot {
    type_unsigned_64 sequence;
    type_status status;
    int valid;
} vm_session_start_outcome_snapshot;

/* Creation returns one owned outcome.  The owner externally serializes every
 * publish, capture, clear and destroy operation; a captured snapshot is a
 * copied value that remains valid after later outcome operations. */
type_status vm_session_start_outcome_create(vm_session_start_outcome **out_outcome);
void vm_session_start_outcome_destroy(vm_session_start_outcome *outcome);
/* Invalidates the current result, preserves sequence, and restores OK status. */
void vm_session_start_outcome_clear(vm_session_start_outcome *outcome);
/* Publishes one status, increments sequence, marks the outcome valid, and
 * returns the same status for direct propagation. */
type_status vm_session_start_outcome_publish(vm_session_start_outcome *outcome,
    type_status status);
/* Captures the complete current copied result. */
type_status vm_session_start_outcome_capture(
    const vm_session_start_outcome *outcome,
    vm_session_start_outcome_snapshot *out_snapshot);

#endif
