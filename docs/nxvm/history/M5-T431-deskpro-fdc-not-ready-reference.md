# M5 T431 S1: DeskPro FDC Not-Ready Reference Completion

## Closed Scope

T431 closes one bounded, reference-derived receiver in the DeskPro device-phase
program. Core now owns an immutable copied policy for an execution-phase READ
on an unready selected drive. Model-40 selects `ST0=48h`, `ST1/ST2=00h` through
the existing result/IRQ route; generic PC/AT retains its pre-existing generic
completion.

## Evidence And Proof

The policy labels transient PCjs observation as `reference-derived` and NEC
uPD765 Ready/IRQ/ST0 controller facts as `original`; neither source is retained.
Focused Core, Model-40 and generic-PC/AT tests pass. The Model-40 regression
covers result phase, IRQ6, acknowledgement, DOR reset cancellation and Sense
Interrupt clearing and emits `M5:T431:S1:MODEL40-FDC-NOT-READY:OK`. Serial
current-gate, documentation governance and actual-diff review pass at closure.

## Core/Profile Ownership And Transfer

Core owns completion state, result, IRQ and cancellation. The Model-40 VM
profile only selects the frozen policy. The terminal-observation success test
now requires normal `ST0`, and the repaired helper preserves scan `ST2`.
Physical FDC rates/waveforms, propagation/arbitration and whole DeskPro L3
acceptance remain explicitly open under proposal receiver 2.