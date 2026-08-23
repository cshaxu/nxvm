# M5 VM Media Persistence Failure Hygiene

## Purpose

Repair VM-owned multi-file media save failure handling so sidecar memory,
temporary files, and paired-save state are cleaned up or retained according to
one explicit failure contract.

## Required Scope

Correct the FDD sidecar serialization failure cleanup and audit every caller of
the paired media-save helper for allocation, temporary-path, replacement, and
rollback behavior. Preserve the current raw-IMG and sidecar formats and
successful-save semantics; make failure observable without leaking resources
or falsely reporting an atomic save.

## Dependencies And Completion

Completion requires injected failures across allocation/write/replace stages,
file-system cleanup assertions, a paired-save caller sweep, and current gates.

## Non-goals And Stop Conditions

Do not add asynchronous I/O, a generic filesystem layer, new media formats, or
physical-media semantics. Stop if cross-file atomic replacement cannot be
specified portably by the current host contract; record that boundary instead
of claiming stronger atomicity.
