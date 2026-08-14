# PC/AT Device, Chip, Port, And Bus Completeness Audit

## Purpose

Create the authoritative hardware completeness ledger needed to finish M5 L3
fidelity without treating a few passing DOS paths as board-wide parity.

## Required scope

Inventory every core-owned PC/AT component: CPU-visible buses and ports,
PIC/PIT/RTC, DMA, KBC/AUX/PPI/NMI, VADP display adapters, FDC/ATA, memory and
ROM mapping, IRQ/DRQ/reset routes, and selected optional interfaces.  For each
one record the primary hardware/manual contract, implemented register and
state surface, reset and error behavior, event/timing owner, consumers, gaps,
and the earliest bounded implementation candidate.  Classify absent devices
separately from selected-but-incomplete devices.

The resulting NMI source inventory is the explicit prerequisite of the later
PC/AT NMI source-ownership candidate; this audit does not select or synthesize
an NMI producer itself.

## Non-goals and stop conditions

Do not implement a device merely because another emulator has it, import
firmware, or turn host devices into default backends.  Stop at any controller
without a primary contract or named corpus; record a corpus-gated transfer.

## Evidence standard

Require one indexed hardware ledger with source, route, lifecycle, timing,
and gap disposition for every selected component, plus Queue/TODO links that
leave no unclassified selected device.
