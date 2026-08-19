# M5 Core CPU Instruction Timing Programs

## Purpose

Migrate the retained 8086, 80186, 80286 and 80386DX instruction-timing ledgers
onto the Core timing-plan path. Replace coarse scalar or ad-hoc surcharge
selection with neutral instruction-form/context timing programs that express
documented constants, formulas and deterministic bounded ranges.

## Required Scope

Consume the reconciled CPU rows and retained T357/T359/T360/T361/T362/T363
ledgers. For each admitted form/context, select timing before the sole Core
retirement publication point and retain architectural fault, interrupt,
prefix, string, branch, operand/address-size and mode semantics. A range must
have a recorded deterministic selection rule; an unimplemented or
insufficiently sourced row remains an explicit L2 fallback.

The four existing CPU profiles are the complete universe for this candidate.
Later CPU profiles require a later Core admission.

## Dependencies

Consumes the Core timing-plan boundary and its CPU conformance rows. It
precedes CPU-to-board transaction timing because instruction-internal time and
external completion must have distinct owners.

## Evidence And Completion Standard

Require source-to-program mapping for every frozen CPU row, focused timing and
fault/retirement regressions for every rule kind, cross-profile non-regression,
and a closure audit showing every row is L3, explicit L2 fallback, not
applicable or unsupported. No profile obtains an unlabelled timing default.

## Non-goals And Stop Conditions

Do not reopen instruction semantic correctness already accepted by T401, add
80486/P5, conflate memory/I/O waits with instruction cost, infer undocumented
physical timing, or perform L4 work. Transfer a source conflict to the master
ledger receiver rather than guessing a cost.
