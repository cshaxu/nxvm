# PC/AT Device Service-Timing Corpus

## Purpose

Turn the device-completeness ledger into source-backed service timing for the
selected PC/AT profile after bus ownership and availability are defined.

## Required scope

For every selected device with externally visible progress, model one owner for
command acceptance, busy/ready transitions, DRQ/IRQ assertions, data phases,
acknowledgement, reset/cancellation, and deterministic event ordering.  Start
with PIT/PIC/RTC, DMA, KBC/AUX/PPI/NMI, VADP, FDC, and ATA; split a device when
its manual or corpus establishes an independent state machine.  Preserve the
existing transaction and copied-host-boundary contracts.

## Non-goals and stop conditions

No speculative delays, analog audio/video, host wall-clock pacing, or generic
device framework.  Stop if required wait states, command phases, or corpus
inputs lack a primary contract; transfer that exact gap rather than inventing a
latency.

## Evidence standard

Require per-device state/timing ledgers, command-to-IRQ/DRQ traces, reset and
cancel proof, port/memory/IRQ/DMA publication checks, and selected firmware or
DOS consumers.  The work is a prerequisite for, not a substitute for,
cycle-exact execution.
