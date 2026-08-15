# DeskPro 386 Model 40 Board Bus And Device Timing Closure

## Purpose

Give the exact DeskPro Model 40 profile, after its selected devices are
functionally complete, one source-labelled board availability and device-service
contract. This is the fourth bounded DeskPro candidate and cannot select or
alter CPU semantics or repair an incomplete device.

## Required scope

Map Model-40 board memory/ROM and I/O availability, DMA arbitration,
PIC/PIT/RTC, keyboard, selected display and storage, NMI producers/latches/
masks, reset and cross-device event order. For each already functional selected
component, establish board-local command availability, ready/busy, DRQ/IRQ,
acknowledgement, cancellation and reset timing through the existing core
transaction/event owners. Use Compaq, Intel and component-primary material plus
project-owned probes; secondary emulators may only cross-check a
primary-constrained result.

## Non-goals and stop conditions

No generic PC/AT substitution, CPU timing repair, device functional repair,
486 inference, physical waveform claim, unselected adapter, Windows claim,
firmware/media import or host-time coupling. Stop and transfer any route whose
selected board or corpus cannot identify an owner and timing disposition.

## Evidence standard

Require a Model-40 board/device source-to-model ledger, contention and reset
replay, command-to-IRQ/DRQ traces, full current-gate proof and a complete
handoff to the DeskPro integration audit.
