# M5 Core CPU Instruction Timing Programs

## Purpose

Migrate the retained 8086, 80186, 80286 and 80386DX instruction-timing ledgers
onto the Core timing-plan path. Replace coarse scalar or ad-hoc surcharge
selection with neutral instruction-form/context timing programs that express
documented constants, formulas and deterministic bounded ranges.

## Shared Admission Baseline

This candidate applies the
[Core specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md)
through the plan boundary and consumes the earliest CPU-program batch:
`CPU-EXEC`, `CPU-EXCEPT`, and `CPU-FPU` from the
[T433 S6 Core L3 admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md).
The [T433 S7 source-sufficiency ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md)
distinguishes a missing Core form selector from a board/transaction input; this
candidate may not turn either into a profile-local algorithm or guessed value.
It owns instruction-form and delivery/FPU rule admission, not board
availability, prefetch production, INTA waveform or profile-local algorithms.
Those retain their ledger-assigned receivers and explicit L2 status until
their own contract is implemented.

## Evidence-First Completion Method

This candidate follows the binding [Td S121 CPU timing governance record](../etc/evidence/td-s121-cpu-timing-evidence-first-governance.md). It is a three-phase task, in this order:

1. freeze a complete 8086/80186 successful-retirement evidence and model ledger before inspecting current timing values;
2. audit the current decoder, timing selectors, retirement publisher, accounting and tests against that ledger, then publish one finite repair plan; and
3. implement declared ledger batches, remove obsolete timing paths, and perform a final full-ledger closure audit.

For an Intel range-only row, a version-pinned, same-CPU reference model that conforms to manual semantics and bounds is a labelled L2 model, never an L3 formula. A manual/reference conflict is resolved in favour of manual semantics while retaining only separable reference timing mechanics. If no range exists, the ledger follows the same reference, corrected-model and project-model ladder; only a row for which no compliant explainable model can be established is an explicit L1 exception. A midpoint is the final bounded L2 fallback, not a hidden default; no successful row may retain an anonymous one-tick result.

## Required Scope

Consume the reconciled CPU rows and retained T357/T359/T360/T361/T362/T363
ledgers. For each admitted form/context, select timing before the sole Core
retirement publication point and retain architectural fault, interrupt,
prefix, string, branch, operand/address-size and mode semantics. A range or
source gap must first receive the Td S121 evidence/model disposition. An
insufficiently supported row is explicit L1 only when that ladder exhausts;
it may not be hidden in a generic fallback.

The four existing CPU profiles are the complete universe for this candidate.
Later CPU profiles require a later Core admission.

## Dependencies

Consumes the Core timing-plan boundary and its CPU conformance rows. It
precedes CPU-to-board transaction timing because instruction-internal time and
external completion must have distinct owners.

## Evidence And Completion Standard

Require source-to-program mapping for every frozen CPU row, focused timing and
fault/retirement regressions for every rule kind, cross-profile non-regression,
and a closure audit showing every row is L3, labelled L2, explicit L1, not
applicable or unsupported. No profile obtains an unlabelled timing default.

## Non-goals And Stop Conditions

Do not reopen instruction semantic correctness already accepted by T401, add
80486/P5, conflate memory/I/O waits with instruction cost, infer undocumented
physical timing, or perform L4 work. Transfer a source conflict to the master
ledger receiver rather than guessing a cost.
