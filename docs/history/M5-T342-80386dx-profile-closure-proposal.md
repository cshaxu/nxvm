# 80386DX Profile Closure

## Objective

Audit the completed 80386DX form and state ledgers against the Intel 80386
manuals and the project closure map. This is a verification candidate, not a
place to implement a missing mechanism.

## Dependency And Method

It runs only after the width/integer and system-state candidates close. It
checks the current source graph, metadata, focused evidence, mode/privilege
matrices, `LOCK`/prefix classifications, and all transfers. It distinguishes
Intel-required 16/32-bit structure from accidental construction divergence.

The audit compares the source inventory, Intel row ledger, focused evidence,
and transfer records in that order. It produces a single residual table whose
only valid outcomes are accepted evidence, precise architectural rejection,
Intel-required retained difference, or return to a named implementation
owner. It does not introduce a catch-all cleanup S.

## Audit Outputs

Each audited form/state row has exactly one result: accepted proof,
Intel-required retained difference, explicit external/post-80386 boundary, or
a return to the exact form/state mechanism owner. A return records its source
route and missing mode, prefix, or fault proof; it never becomes an audit-side
patch.

## Non-goals And Stop Conditions

It does not repair a missing handler, state transition, or test matrix. Any
gap returns to the earliest owning 80386DX implementation candidate (or the
earlier profile owner if classification proves that), with an exact admission
reason. It cannot mark a row complete merely because a legacy smoke happens to
execute it.

## Exit Standard

Every 80386DX-assigned row is implemented and proved, explicitly rejected,
Intel-required and deliberately retained, or transferred once to an approved
outside boundary. No partial, missing, or unclassified row remains before
four-profile cross closure.
