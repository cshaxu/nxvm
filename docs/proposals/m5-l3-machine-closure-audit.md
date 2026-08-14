# M5 L3 Machine Closure Audit

## Purpose

Audit the complete IBM PC/AT 5170 80286 baseline after timing, bus, and device
work to prove one coherent, source-labelled model-L3 machine. The later 80386
DeskPro 386, then 8088/5150/XT baseline closures remain prerequisites before
Windows is used as a
compatibility consumer.

## Required scope

Reconcile the 5170 instruction-timing, bus-availability, device-service, reset,
IRQ, DMA, memory/ROM, and profile ledgers. For each selected hardware owner,
prove that its source contract, publication order, fault/cancel path, and
deterministic trace agree. Identify every unselected device, analog behavior,
model-derived row, and reference-exhausted physical timing as an explicit
transfer.

## Non-goals and stop conditions

This is an audit, not a place to hide implementation work.  Any reproduced
defect returns to its earliest owner task. It does not claim pin-level
waveforms, physical cycle exactness, every historical clone, x87, or Windows
compatibility.

## Evidence standard

Require a component-by-component closure matrix, cross-device ordering replay,
reset/fault/cancellation evidence, full current gate, and an explicit 5170-L3
ready/not-ready decision with all residual transfers. This closure is a
prerequisite, not a substitute, for the later DeskPro 386, then 5150/XT 8088
baseline closures.
