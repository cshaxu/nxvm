# Specification-Driven Simulation Admission Detail

`M5:Td:S114:HARDWARE-TIER-LEDGER:OK`

This supporting record supplies the reusable ledger shape for the shared
[complete simulation admission proposal](../../proposals/m5-specification-driven-complete-simulation-admission-program.md).
It is not a Rule, task packet, machine profile, or competing architecture.

## Required hardware-unit ledger

Every admitted hardware T records one finite row for every hardware unit in
its scope. A unit may be a CPU form/context, controller command/phase, board
route, timing domain, device register/mode, media operation or display state.

| Field | Required record |
| --- | --- |
| Unit and configuration | Neutral unit name plus selected CPU/chip/device revision, board route, clock/wiring or media personality when applicable. |
| Evidence level | `document-primary`, `reference-derived`, or `maintained-boundary`. |
| Evidence | Exact manual/reference location; Level 2 also names version, configuration, input and project-owned replay; Level 3 records the bounded absence finding. |
| Rule and implementation | Functional state rule, timing/phase rule if sourced, producer/consumer route, and the one Core/VM owner. |
| Lifecycle | Initialize, assertion/completion, acknowledgement, cancellation and reset behavior. |
| Proof | Focused test/probe/trace, regression owner and expected observable checkpoint. |
| Closure disposition | Implemented at Level 1 or 2, or maintained at Level 3 with its explicit non-L3 claim and future receiver. |

## Classification procedure

1. Inspect applicable checkable documentation before looking to a reference
   emulator. Documented behavior is Level 1 and must be implemented for the
   unit's declared supported surface.
2. For a bounded uncovered observable, create a Level-2 reference contract
   only after recording a named emulator/version/configuration and a
   project-owned reproducible observation. Resolve contradictions in favor of
   the documentation or block for reconciliation.
3. If neither produces an admissible rule, implement only the deterministic
   safe behavior necessary for the declared surface. Record it as Level 3,
   including reset/cancellation and the exact non-claim.

No ledger row is complete merely because it is classified: the task must carry
out and prove the action demanded by its level. The ledger is reviewed again at
task closure so a later regression cannot hide a remaining unit.

## Boundary

Host event arrival, copied presentation, debugger inspection and session
commands can be hardware-task context but are not chip timing units. A task
records their isolation/lifetime proof rather than fabricating a manual timing
rule. This does not weaken the requirement to implement all guest-visible
hardware units in the task's frozen scope.
