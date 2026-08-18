# DeskPro 386 Board-Level Timing Closure

## Purpose

Implement and verify the selected 1986 Compaq DeskPro 386 Model 40 board-clock
and bus-timing contract only after the preceding 80386 physical-retirement
qualification accepts the selected corpus. This candidate is separate from
per-device/firmware service timing and the final Model-L3 audit.

## Required Scope

Consume the accepted shared CPU qualification and establish source-backed
Model-40 contracts for:

- 16 MHz CPU-related time and 1.19318 MHz system/auxiliary 8254 TIMCLK,
  including rational conversion and reset phase;
- DCLK/BCLK availability, dual-8237A DMA arbitration and documented DMA wait
  placement; and
- board-level order/visibility among dual PITs, dual PICs, RTC/CMOS, 8042,
  port `61h` refresh/failsafe/I/O-channel status, reset and NMI producers.

Core owns shared time, clock-domain, transaction, DMA/PIC/PIT and reset
mechanisms. VM owns the selected Model-40 frequency/topology declaration. Any
D4-specific declaration uses these owners, not a private timer, second
scheduler or generic-PC/AT substitute.

Use Compaq D3PE and component-primary material first. If it states only a
range or omits observable phase, a bounded primary-constrained observation may
use 86Box, MAME, PCjs, Bochs or a project-owned probe; it cannot turn a generic
AT configuration into Model-40 authority.

## Non-goals And Stop Conditions

No CPU instruction-timing/qualification repair, storage/FDC/HDC/CECG firmware
service work, physical-media representation, analog/pin waveform, ROM/media
import, host wall-clock coupling or Model-L3 decision. A device command
latency or BIOS-visible service state transfers to the following
physical-device/firmware candidate. If the prerequisite corpus is not
physical-qualified, stop rather than publish a scalar or local delay.

## Evidence And Completion Standard

Require the accepted qualification input, primary source-to-Core/VM ledger,
rational-clock/reset regressions, DMA/IRQ/NMI/order traces, consumer and
similar-issue sweeps, and full current-gate verification. The closure must
identify every remaining device/firmware timing receiver before the next
DeskPro task begins.