# DeskPro 386 Board-Level Timing Closure

## Purpose

Implement and verify the exact Model 40 board-clock and bus-timing contract
once the shared CPU physical-timebase closure makes CPU execution eligible to
advance documented clock domains. This is a board-level task, separate from
per-device/firmware service timing and from the final Model-L3 audit.

## Required Scope

Consume the T388 shared timebase boundary and establish source-backed Model 40
contracts for:

- 16 MHz CPU-related time, 1.19318 MHz system and auxiliary 8254 TIMCLK, and
  their rational conversion/reset behavior;
- DCLK/BCLK availability, dual-8237A DMA arbitration and the documented DMA
  wait placement; and
- board-level ordering/visibility among dual PITs, dual PICs, RTC/CMOS, 8042,
  port `61h` refresh/failsafe/I/O-channel status, reset and NMI producers.

Core owns the shared time, clock-domain, transaction, DMA/PIC/PIT and reset
mechanisms. VM owns the selected Model-40 frequency/topology declaration.
Any D4-specific declaration must use those owners rather than a private timer,
second scheduler or generic-PC/AT substitute.

Use Compaq D3PE and component-primary material first. If it states only a
range or omits an observable phase, a bounded, primary-constrained observation
contract may use 86Box, MAME, PCjs, Bochs or a project-owned probe; it must not
turn a generic AT configuration into Model-40 authority.

## Non-goals And Stop Conditions

No CPU instruction-timing repair, storage/FDC/HDC/CECG firmware service work,
physical-media representation, analog/pin waveform, ROM/media import, host
wall-clock coupling or Model-L3 decision. A device command latency or BIOS
visible service state transfers to the following physical-device/firmware task.

## Evidence And Completion Standard

Require a primary source-to-Core/VM ledger, rational-clock/reset regressions,
DMA/IRQ/NMI/order traces, consumer and similar-issue sweeps, and current-gate
verification. The closure must identify every remaining device/firmware timing
receiver before the next DeskPro task begins.