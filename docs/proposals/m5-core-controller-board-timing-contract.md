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

## Planned Subtasks

1. **S1 - finite ledger and contract design.** Reconcile every retained
   PIC/DMA/PIT L2 row with actual owners and callers; define the minimum value
   vocabulary, validation matrix, source/provenance record and migration set.
   No runtime change.
2. **S2 - one copied plan boundary.** Implement and validate the smallest
   immutable board-timing input at the existing Core configuration boundary;
   migrate existing clock/transaction inputs without a parallel legacy path.
3. **S3 - controller consumption and proof.** Bind the admitted PIC/DMA/PIT
   values at their current owners, delete superseded plumbing, and prove route,
   phase, reset and rejection behaviour using the frozen ledger.
4. **S4 - closure and profile-export audit.** Verify provenance preservation,
   no profile-to-device reverse dependency, all ledger dispositions, current
   gate and artifact; transfer any unqualified board/electrical facts rather
   than encoding guesses.

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
