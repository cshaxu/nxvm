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

## Implementation S Decomposition

The task is decomposed by the timing inputs and architectural paths that must
be observed, not by arbitrary opcode ranges. Each implementation S owns the
listed canonical manifest partition, its focused functional and retirement
regressions, and removal or disposition of any superseded successful path.
Every S re-runs the manifest and decoder-ledger verifiers; no S may mark a
key conforming until it has a real result-contract observation.

| S | bounded owned result | exit criterion |
| --- | --- | --- |
| S1 | Reconcile the transferred 807-key universe with the current source, decoder inventory, test registration and result contract; freeze an executable partition map and the exact per-S key ownership before runtime changes. | Every base, single-axis and combined key has exactly one later S owner; all 47 missing-input and 239 missing-test facts are reproducible; no key is silently excluded or duplicated. |
| S2 | Complete the result-contract producer and sole 80286 publication seam so focused executions can emit canonical key IDs, origin, ticks, normalized inputs and unallocated status without manufacturing conformance. | The runner materializes every key and fails honestly for uncovered ones; a focused probe can emit a valid record for each input shape; no parallel successful-retirement publisher remains. |
| S3 | Implement and observe non-control instruction forms: arithmetic, adjust, Group-2/Group-3, moves, exchange, stack, flags, BOUND, ARPL, ports and FPU-interface instruction-side rows, including r/m, EA, odd-word and legal memory-RMW LOCK inputs. | Each owned exact/formula key has functional and timing results with its Appendix-B inputs; memory/address variants have no fallback or unallocated retirement. |
| S4 | Implement and observe strings, port strings, repeat phases, segment overrides and word odd-address composition. | Every owned primitive, repeat, zero-count, continuation, override and odd-word key has a canonical L3 result and its actual iteration/phase inputs. |
| S5 | Implement and observe ordinary control transfer: conditional outcomes, LOOP/JCXZ, near/far calls and jumps, real-mode returns, interrupts and next-instruction-byte terms. | Every owned outcome, direct/indirect and real-mode next-byte key has a functional result and the exact manual formula inputs. |
| S6 | Implement and observe protected control transfer: same/outer privilege calls, returns and IRET, call/gate/task paths and their next-byte terms. | Every owned protected path has a real successful execution, normalized privilege/path and next-byte inputs, exact L3 timing and no downgrade to a real-mode or generic path. |
| S7 | Implement and observe protected segment, descriptor and system forms: segment loads/stores, LDS/LES, LAR/LSL, VERR/VERW, descriptor-table, machine-status, LDT and task-register forms. | Every owned legal protected register/memory form has focused functional proof and canonical L3 timing; invalid/faulting forms remain outside the successful-retirement universe. |
| S8 | Run the complete generated-key result closure, cross-profile non-regression, source/decoder sweep and task-level cleanup/audit; resolve any residual key only within its originating S boundary or transfer it explicitly. | All 807 keys are L3 conforming with zero wrong-value, unallocated, missing-input and missing-test; one publisher remains; the developer artifact, full regressions and task-level audit pass. |

The S boundaries intentionally place every timing-sensitive protected path in
S6 or S7, rather than allowing a broad fallback to claim coverage. S8 is a
closure/audit S, not a place to hide unimplemented instruction families.

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
