# M5 80386DX Instruction Timing Closure

## Purpose

Close the complete 80386DX successful-retirement instruction-timing universe
as the second, independently admitted Core implementation candidate. It starts
only after the preceding 80286 closure has produced its bounded transfer.

## Admission Baseline

The authoritative baseline is the 80386DX
[manual ledger](../etc/cpu-timing/t435-s1-80386-ledger.md), generated
[timing manifest](../etc/cpu-timing/t435-s2-80386-timing-manifest.json),
[implementation tracker](../etc/cpu-timing/t435-s2-80386-implementation-tracker.md),
[implementation audit](../etc/cpu-timing/t435-s2-80386-implementation-audit.md),
and the decoder-ledger evidence named by those records. The frozen universe is
450 all-L3 base forms plus 961 finite legal size, repeat-phase, multiplier,
segment and memory-RMW LOCK contexts: 1,411 canonical keys.

The transferred audit is deliberately nonconforming: 257 keys lack focused
results, 192 dynamic/path/privilege keys lack normalized inputs, and `I386-XLAT`
is unallocated. The candidate must replace that audit state with per-key
result-contract proof; neither selector presence nor aggregate smoke is
completion evidence.

## Required Scope

Implement every manifest key through the sole Core-private timing-selection
and retirement-publication route. Scope includes compatible and 80386DX forms,
`0F` extensions, operand and address-size variants, real, protected and VM86
successful paths, privilege and task/gate paths, control transfer next-component
terms, repeat/string phases, data-dependent early-out multiply, r/m forms,
legal prefixes and LOCK composition, and successful I/O/system forms. Each
result must carry its Chapter-17 origin, ticks, formula inputs and
`source_timing_unallocated=false`.

This candidate preserves accepted instruction semantics. It excludes external
READY/HOLD/BUSLOCK arbitration, prefetch and bus waits, device response,
physical board timing, exception/event delivery, public ABI changes, source
import, and new CPU-profile work.

## Completion Standard

Admission must freeze a finite implementation breakdown and regressions before
code changes. Closure requires the generated-key result verifier and
decoder/manifest verifiers to report all 1,411 keys L3 conforming with zero
wrong-value, unallocated, missing-input and missing-test states. One publisher
must remain; obsolete successful fallback paths must be removed or
dispositioned; focused functional/fault and relevant cross-profile regressions
must pass; and a task-level evidence and actual-change audit must accept the
result.

## Dependencies And Stop Conditions

This candidate follows the 80286 closure candidate and precedes CPU-to-board
timing work. A manual or manifest contradiction, unrepresentable approved-boundary
input, necessary public ABI or board-timing change, source-policy issue, or
verifier contradiction stops implementation for coordinator disposition. It
must never guess a cost, silently reduce an L3 rule, or retain a generic
successful fallback.
