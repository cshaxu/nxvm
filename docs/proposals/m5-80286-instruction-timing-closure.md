# M5 80286 Instruction Timing Closure

## Purpose

Close the complete 80286 successful-retirement instruction-timing universe as
one bounded Core implementation candidate. This is the first M5 candidate and
is intentionally independent of the later 80386DX closure candidate.

## Admission Baseline

The authoritative source and coverage baseline is the 80286
[manual ledger](../etc/cpu-timing/t435-s1-80286-ledger.md), the generated
[timing manifest](../etc/cpu-timing/t435-s2-80286-timing-manifest.json), the
[implementation tracker](../etc/cpu-timing/t435-s2-80286-implementation-tracker.md),
the [implementation audit](../etc/cpu-timing/t435-s2-80286-implementation-audit.md),
and the 80286 decoder inventory and verifiers named by those records. The
frozen universe is 286 all-L3 base forms plus 521 finite legal context and
combined keys: 807 canonical successful-retirement keys.

The prior program transferred a partial foundation, not a closure result. Its
latest observed runner had 606 records, while the manifest verifier reported
`conforming=0`, `wrong=0`, `unallocated=0`, `missing_input=47`, and
`missing_test=239`. This candidate must independently reproduce and replace
that partial state with canonical per-key closure evidence; it receives no
implicit completion credit.

## Required Scope

Implement every manifest key through the single Core-private timing-selection
and retirement-publication route. It includes real and protected-mode results,
privilege, gate and task paths, effective-address and odd-word adjustments,
next-instruction-byte terms, outcomes, repeat/string phases, shifts, legal
LOCK composition, I/O-success paths, and descriptor/system forms. Every
successful form must expose the inputs needed by its Appendix-B rule and emit
ticks, origin, normalized inputs and `source_timing_unallocated=false`.

The candidate preserves accepted instruction semantics and does not own READY,
HOLD, BUSLOCK arbitration, prefetch availability, memory or I/O wait states,
DMA, device response, physical board cycles, exception/event delivery, public
ABI changes, source import, or a new CPU profile.

## Completion Standard

Admission must freeze one finite implementation breakdown and focused
regressions before code changes. Closure requires the generated-key result
verifier and decoder/manifest verifiers to report every 80286 key as L3
conforming, with zero wrong-value, unallocated, missing-input and missing-test
keys. The implementation must retain one publisher, remove or disposition
obsolete successful fallback paths, pass focused function/fault and relevant
cross-profile regressions, and receive a task-level evidence and actual-change
audit.

## Dependencies And Stop Conditions

This candidate consumes the retained CPU timing foundation and source ledgers;
it precedes the 80386DX candidate and CPU-to-board timing work. A manual or
manifest contradiction, a required input that cannot be represented at the
approved Core boundary, a necessary public ABI or board-timing change, a
source-policy issue, or a verifier contradiction stops implementation for
coordinator disposition. No rule may be guessed, silently downgraded, or
hidden in a generic fallback.
