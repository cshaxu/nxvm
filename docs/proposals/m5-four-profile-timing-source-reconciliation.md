# Four-Profile Intel Timing Source Reconciliation

## Purpose

Reconcile the timing authority for every selected 8086, 80186, 80286, and
80386DX instruction row before it is used by the complete timing corpus or a
cycle-exact profile.  The existing 80286 `NOP` Appendix-B/prose disagreement
is the first known case, not a claim that the other three profiles are closed.

## Required scope

Build a source ledger per profile: processor edition/stepping contract,
instruction form, addressing and privilege assumptions, table/prose values,
and the project timing consumer.  Resolve known contradictions from primary
Intel material, then sweep every timing row that cites the affected source.
Where primary material remains ambiguous, retain the formally selected table
value, record the ambiguity, and prevent a secondary emulator or host benchmark
from becoming authority.

## Non-goals and stop conditions

This work does not implement missing instruction semantics, bus waits, device
latencies, or cycle exactness.  Stop on a source ambiguity that cannot be
resolved from bounded primary material; transfer it as a precise TODO instead
of averaging values or inferring a profile-wide answer.

## Evidence standard

Require a four-profile source/caller ledger, exact primary citations, every
affected timing-consumer disposition, focused regression for every changed
allocation, and a retained unresolved-source record.
