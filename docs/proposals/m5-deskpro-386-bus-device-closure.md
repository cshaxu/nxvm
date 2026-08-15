# DeskPro 386 Board Bus And Device Timing Closure

## Purpose

Give the exact DeskPro profile selected by the preceding CPU candidate one
source-labelled board availability and device-service contract.  This is the
second bounded DeskPro candidate and cannot select or alter CPU semantics.

## Required scope

Map board memory/ROM and I/O availability, DMA arbitration, PIC/PIT/RTC,
keyboard, selected display and storage, NMI producers/latches/masks, reset and
cross-device event order.  For each selected component, establish command,
ready/busy, DRQ/IRQ, acknowledgement, cancellation and reset publication
through the existing core transaction/event owners.  Use Compaq, Intel and
component-primary material plus project-owned probes; secondary emulators may
only cross-check a primary-constrained result.

## Non-goals and stop conditions

No generic PC/AT substitution, CPU timing repair, 486 inference, physical
waveform claim, unselected adapter, Windows claim, firmware/media import or
host-time coupling.  Stop and transfer any route whose selected board or corpus
cannot identify an owner and timing disposition.

## Evidence standard

Require a board/device source-to-model ledger, contention and reset replay,
command-to-IRQ/DRQ traces, full current-gate proof and a complete handoff to
the DeskPro integration audit.
