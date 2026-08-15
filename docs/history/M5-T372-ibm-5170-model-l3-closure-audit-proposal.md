# IBM PC/AT 5170 Current-State Closure Audit

## Purpose

Audit the current IBM PC/AT 5170 80286 closure evidence and record the
truthful ready/not-ready decision. It completes the present audit package; it
does not presume that the Model 339 is L3-ready. A later, fresh final-5170
audit follows the capability ledger, selected-device functional closure, and
board/phase-timing closure. The later DeskPro Model 40 and PC/XT 5160-268
closures remain prerequisites before Windows is used as a compatibility
consumer.

## Required scope

Reconcile the available 5170 instruction-timing, bus-availability,
device-service, reset, IRQ, DMA, memory/ROM, and profile ledgers. For each
selected hardware owner, identify what its source contract, publication order,
fault/cancel path, and deterministic trace do and do not establish. Transfer
each outstanding functional, board/phase-timing, unselected-device, analog,
model-derived, and reference-exhausted boundary to the ordered later receiver.

## Non-goals and stop conditions

This is an audit, not a place to hide implementation work.  Any reproduced
defect returns to its earliest owner task. It does not claim pin-level
waveforms, physical cycle exactness, every historical clone, x87, or Windows
compatibility.

## Evidence standard

Require a component-by-component current-state matrix, cross-device ordering
replay where already supported, reset/fault/cancellation evidence, applicable
current-gate evidence, and an explicit 5170-L3 ready/not-ready decision with
all residual transfers. A not-ready outcome is valid and is expected unless
the later functional and phase-timing requirements already have evidence. This
audit is a prerequisite, not a substitute, for the later 5170 final audit,
DeskPro Model 40, and PC/XT 5160-268 closures.
