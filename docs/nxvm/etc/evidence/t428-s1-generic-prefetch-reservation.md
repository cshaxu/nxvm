# T428 S1: Generic-AT Prefetch Reservation Lifecycle

`M5:T428:S1:GENERIC-PREFETCH-PRODUCER:OK`

## Evidence Tier And Boundary

Tier: `generic-at`. No original DeskPro material or controller-matched reference
was found that defines an asynchronous 80386 instruction-prefetch producer
phase which may safely be modeled in this Core. This receiver therefore adds
only the bounded state and arbitration boundary needed by a later admitted
producer. It does not treat a logical adjacent read as physical overlap.

## Delivered Owner Contract

Core owns a Model-40-selected prefetch reservation with linear identity and
bounded byte count. A successful real-mode instruction round may reserve the
remaining sequential window only when the existing prefetch queue is valid and
within its segment limit. The shared arbitration owner releases that reservation
only after refresh, DMA request/grant, transaction ownership, and HOLD have had
priority; a refresh transaction consumes its whole arbitration tick. The
reservation is cleared by reset, stopped-state invalidation, control-flow queue
mismatch, and CR0/CR3 translation changes.

The reservation does not read memory, open a second CPU transaction, deliver an
exception, publish an external-cycle phase, declare overlap, add timing, or
replace existing instruction fetch. Existing instruction execution remains the
sole memory-transaction and exception owner. Model-40 selects the generic
skeleton; IBM 5170 Model 339 leaves it disabled. VM adds no scheduler or
transaction path.

## Verification And Sweep

`core-machine-prefetch-locality-smoke` proves bounded creation, arbitration
release, invalidation, reset, DMA HOLD priority, same-tick refresh priority and
absence of an external-cycle overlap or timing result. The retained marker is
`M5:T428:S1:GENERIC-PREFETCH-PRODUCER:OK`; it labels the state-producer
foundation, not a physical producer. `vm-model40-private-composition-s7-smoke`
proves selection and `vm-ibm-5170-model-339-composition-smoke` proves
isolation. The sweep covers prefetch refill/invalidation, CR0/CR3, successful
instruction, reset, HOLD, refresh, transaction owner and both selected profiles.

## Transfer

A source-backed asynchronous producer still requires a specified external-cycle
phase, cancellation and exception contract before it may fetch memory or emit
`OVERLAP_DECLARE`. D4 page hit, BWAIT waveform, ISA availability waits, PAL
row/bank decode, scalar calibration and DeskPro physical/L3 acceptance remain
transferred.