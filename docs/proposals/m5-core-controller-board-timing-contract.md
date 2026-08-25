# M5 Core Controller Board-Timing Contract

## Purpose

Establish one immutable, profile-to-Core board-timing contract for the closed
PIC 8259A, DMA 8237A and PIT 8254 owners.  The contract lets a selected board
provide source-backed L3 timing/topology values, or an explicitly labelled L2
proportional fallback, without profile callbacks, controller pointers, a second
scheduler or a second copy of controller state.

An exact calculation from an L2 assumption is still an L2 board premise.  It
may drive deterministic L3 chip transitions inside Core, but it may not be
reported as end-to-end board L3.  Only a qualified board source plus its
declared conversion can produce a board-L3 claim.

## Admission Baseline And Dependencies

Consumes the closed PIC, DMA and PIT ledgers: T456's PIC-T2 logical-latency
boundary, T460's IBM AT 3 MHz/five-clock boundary, and T461 P15--P18.  It
extends the existing `core_machine_config` clock and transaction inputs toward
the copied Core-plan boundary in the
[specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md).
It does not reopen controller chip state machines or invent a physical-board
source.

Before implementation, S1 freezes a finite three-controller exception ledger:
each row names the capability, selected Core owner, current behaviour, missing
fact, evidence tier, regression, risk, and exactly one receiver.  A row is
either a source-backed L3 input, an explicit L2 fallback, not applicable, or
unsupported.  No row may be silently promoted by a guest boot or by arithmetic.

## Required Scope

1. Define the smallest neutral copied-value contract that can carry only these
   established kinds of board input: rational clock ratio plus reset phase,
   selected logical route/topology, and a source-defined discrete controller
   service or visibility term.  Reuse and consolidate the existing clock-plan
   and transaction-plan ownership rather than creating parallel timing plans.
2. Validate the contract before Core construction, copy it into the Core-owned
   machine plan, and reject incoherent ratios, routes, capabilities and values.
   Profile code supplies immutable values only; it never holds a device pointer
   or mutates a controller after construction.
3. Make PIC, DMA and PIT consume their own selected values only through their
   existing Core owner and the existing ordered time/transaction path.  PIT
   retains its OUT/GATE boundary, DMA its transaction/arbitration owner, and
   PIC its IRQ/acknowledgement owner.  Remove any replaced configuration route
   in the same change.
4. Give the finite ledger a uniform exception record and evidence rule.  A
   source-backed value is eligible for a board-L3 result only with a named
   primary/manual or admitted reference-derived source, formula/conversion,
   lifecycle semantics and focused proof.  An L2 input remains visibly L2
   through profile resolution, Core validation and closure evidence even when
   its integer conversion produces exact deterministic Core ticks.
5. Add focused immutable-plan, invalid-plan, reset/cancellation, route and
   controller-order regressions.  Test both a source-backed fixture and an L2
   fallback fixture where each is admitted; neither fixture may depend on host
   wall time.  Complete the current gate and the required current Release
   artifact after the implementation task is admitted.

## Contract Shape And Ownership

The contract is construction input, not a runtime timing-setter surface.  A
profile resolver writes neutral values once; Core validates and copies them;
the existing shared timeline advances the existing controller owner.  A profile
never changes a selected ratio, delay or route after construction.  Rebuilding
or resetting a machine selects a new validated contract; it does not mutate a
live controller through a profile callback.

The implementation must first determine whether the existing clock-plan and
transaction-contract fields can be consolidated directly.  If a new nested
value is necessary, it replaces rather than shadows the affected existing
input.  It contains only typed values and neutral contract IDs, never a
machine name, controller pointer, function pointer or generic `delay` field.

| Owner | Eligible construction fields | Existing runtime inputs retained | Prohibited shape |
| --- | --- | --- | --- |
| PIT | Input-clock rational ratio, reset phase, selected OUT route and selected GATE-source contract IDs. | `core_machine_pit_set_gate` remains a dynamic board signal; `pit.c` retains count/mode/OUT state. | A live frequency/delay setter, profile-owned PIT state or a second PIT tick loop. |
| DMA | Input-clock rational ratio, admitted service-phase contract ID and admitted refresh/DRQ route ID; selected values must feed the existing transaction/arbitration contract. | DREQ, EOP, BUSRDY and transaction lifecycle remain dynamic inputs at their current owners. | A per-transfer cycles setter, direct board mutation of DMA phase, or a DMA-local scheduler. |
| PIC | Programmed cascade/IRQ route ID and, only if source-backed with a Core tick meaning, a discrete request-visible contract ID. | IRQ source assertion/deassertion and PIC acknowledgement remain dynamic inputs at their current owners. | A naked nanosecond/delay setter, a clocked PIC loop without a source-backed contract, or profile-owned IRR/ISR state. |

The existing Core elapsed-tick path is the only timing driver:

`CPU retirement/timeline -> validated clock or transaction term -> controller
owner -> existing route/observation`.

An L2 ratio may be normalized outside Core into an exact integer or rational
value so that the owner executes deterministically.  Its profile provenance
and the selected fallback contract remain L2; Core observations must not label
the resulting board interval as source-backed L3.  A source-backed L3 value
must name its rule, conversion, lifecycle and focused proof in the S1 ledger.

## Planned Subtasks

1. **S1 - finite ledger, field vocabulary and migration decision.** Reconcile
   every retained PIC/DMA/PIT L2 row with actual owners and callers.  For every
   prospective field, record its unit, source/provenance, validator, consumer,
   reset/cancellation semantics and regression.  Decide field-by-field whether
   existing clock/transaction input absorbs it or one replacement nested value
   is needed.  No runtime change.
2. **S2 - one copied construction-plan boundary.** Implement the accepted
   typed values and contract IDs at the existing Core configuration boundary;
   validate/copy them before machine construction and delete superseded input
   plumbing.  Prove that invalid ratios, phases, routes and contract IDs reject
   before publication, with no profile callback or device-pointer escape.
3. **S3 - PIT and DMA owner consumption.** Bind PIT's clock/route/GATE contract
   and DMA's clock/service/refresh contract at their existing owners.  Prove
   the shared-timeline path, route selection, service/reset/cancellation and
   explicit L2 fallback provenance; do not add a PIT/DMA setter or scheduler.
4. **S4 - PIC consumption and whole-contract closure.** Bind PIC's selected
   route and any qualified discrete visibility contract through its existing
   acknowledgement path, or retain the current logical L2 fallback where no
   source admits a field.  Audit all ledger dispositions, provenance,
   controller order, profile-direction boundary, current gate and artifact;
   transfer unqualified electrical facts rather than encoding guesses.

## Non-goals And Stop Conditions

This task does not implement L4 electrical waveforms, oscillator measurement,
host pacing, a VM profile resolver, new controller personalities, arbitrary
board wiring, a generic callback framework, or external source import.  Stop
and transfer any value lacking a stable semantic unit, a qualified source, or
a single existing Core owner.  Do not add a placeholder field merely because a
future board might need it.

## Completion Standard

The task closes only when every frozen PIC/DMA/PIT exception row has one
durable disposition and each eligible value follows one direction:

`profile evidence/value -> validated copied Core plan -> existing owner ->
ordered Core observation`.

No Core code may name a machine/profile, no profile may retain Core device
state, no controller may acquire a second clock or scheduler, and no L2 input
may be described as a board-L3 fact.  Closure requires focused proof, full
current-gate, documentation governance, actual-diff review, an updated
stripped Release artifact and a code-size accounting.
