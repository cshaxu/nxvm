# Bus-Timed PC/AT Operation

## Purpose

Extend the accepted T354 transaction owner from synchronous selected
publication to a bounded PC/AT bus-availability model.  This candidate begins
only after instruction-timed execution supplies a guest-time contract.

## Required scope

Select a documented PC/AT profile and hardware/corpus contract.  Define one
owner for CPU memory/I/O availability, named memory and port wait states,
DMA HOLD/HLDA-style exclusion, and selected device ready/DRQ/IRQ visibility.
Use the existing transaction begin/commit/cancel lifecycle rather than a
parallel scheduler or test-only mirror state.  Map validation, reservation,
commit, cancellation, reset and trace behavior across CPU, DMA, PIC, FDC and
HDC; distinguish logical interrupt acknowledgement from physical INTA.

## Non-goals and stop conditions

No universal chipset, electrical waveform, speculative latency, arbitrary
device timing, cache/prefetch, host-time coupling, or broad peripheral
implementation.  Stop if a selected device requires a separate command/model
contract or the PC/AT source cannot identify the required wait/ownership
behavior.

## Evidence standard

Require a source/caller/write and timing-state ledger, contention and reset
proof, transaction trace proof, selected firmware/DOS consumers, direct
current-gate regression, and precise transfers.  It consumes instruction
timing and is the prerequisite for a cycle-exact profile, not a cycle claim.
