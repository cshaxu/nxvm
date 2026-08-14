# Bus-Timed PC/AT Operation

## Purpose

Extend the accepted T354 transaction owner from synchronous selected
publication to the IBM PC/AT 5170 80286 baseline's bounded bus-availability
model. This candidate begins only after instruction-timed execution supplies a
guest-time contract.

## Required scope

Use the owner-approved IBM PC/AT 5170 80286 baseline: a fixed system-board
revision, 6 MHz 80286, 512 KiB motherboard RAM, documented ROM configuration,
360 KiB FDC/DMA channel-2 path, and a project-owned firmware/DOS probe corpus.
The admitting S records the exact board manual edition and every selected ISA
adapter; an unselected adapter cannot supply a timing value. Define one owner
for CPU memory/I/O availability, named memory and port wait states, DMA
HOLD/HLDA-style exclusion, and selected device ready/DRQ/IRQ visibility.
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
proof, transaction trace proof, project-owned firmware/DOS consumers, direct
current-gate regression, and precise transfers. It consumes instruction timing
and is the prerequisite for the 5170 phase and closure candidates, not a cycle
claim.
