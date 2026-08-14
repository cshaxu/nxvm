# M5 L3 Machine Closure Audit

## Purpose

Audit the complete selected PC/AT machine after timing, bus, and device work to
prove one coherent L3 model before Windows is used as a compatibility consumer.

## Required scope

Reconcile the instruction-timing, bus-availability, device-service, reset,
IRQ, DMA, memory/ROM, and profile ledgers.  For each selected hardware owner,
prove that its source contract, publication order, fault/cancel path, and
deterministic trace agree.  Identify every unselected device, analog behavior,
or unknown physical timing as an explicit transfer.

## Non-goals and stop conditions

This is an audit, not a place to hide implementation work.  Any reproduced
defect returns to its earliest owner task.  It does not claim pin-level
waveforms, every historical clone, x87, or Windows compatibility.

## Evidence standard

Require a component-by-component closure matrix, cross-device ordering replay,
reset/fault/cancellation evidence, full current gate, and an explicit M5-L3
ready/not-ready decision with all residual transfers.
