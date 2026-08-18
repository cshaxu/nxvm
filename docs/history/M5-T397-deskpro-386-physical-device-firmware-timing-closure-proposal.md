# DeskPro 386 Physical-Device And Firmware-Timing Closure

## Purpose

Complete the exact Model 40 board-visible timing work which cannot be resolved
by CPU-clock eligibility alone. This candidate consumes the four-profile
physical-timebase closure, the DeskPro board-level timing closure, and the
accepted Model-40 topology/function records;
it precedes the Model-40 L3 audit and is not a generic PC/AT task.

## Required Scope

For every selected Model-40 device and firmware-visible board route, establish
one primary-labelled availability/service contract or retain a precise receiver:

- the selected 1.2 MB FDC/DMA2/IRQ6 and Compaq 40 MB controller/IRQ14 routes,
  including the boundary between logical RAW-IMG service and physical media;
- selected Compaq Enhanced Color Graphics board/firmware timing, including
  remaining monitor/raster/ISA/firmware-visible effects; and
- selected keyboard, reset and NMI paths that have a source-backed timing or
  visibility requirement.

Use Compaq, Intel and component-primary documentation first. When that
material provides a range or no deterministic value, use a bounded,
primary-constrained 86Box, MAME, PCjs, Bochs or project-owned observation only
under an explicit measurement contract. Reuse Core transaction/timeline owners
and VM profile declarations; do not create a Model-40-local scheduler, generic
AT substitute, or host-time device clock.

## Non-goals And Stop Conditions

No CPU instruction-timing repair, physical media representation, pin/analog
waveform, generic 80386 PC conclusion, clone compatibility claim, ROM/media
import, or final L3 decision. A physical hard-drive/ECC/format-track behavior,
unselected CECG breadth, optional keyboard hardware, or unobservable board
phase remains a named TODO or later candidate with an admission condition.

## Evidence And Completion Standard

Require a source-to-owner-to-probe ledger for all selected routes, command to
IRQ/DRQ/NMI traces, reset/cancellation replay, CPU-timebase dependency proof,
and current-gate verification. The final record must distinguish completed
board-visible behavior from physical-media, analog and pin-level transfers.
Only then may the DeskPro Model-L3 audit begin.