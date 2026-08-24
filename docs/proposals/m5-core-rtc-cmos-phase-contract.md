# M5 Core RTC CMOS Phase Contract

## Purpose

Define MC146818 calendar, periodic, alarm, update and Register-C-to-IRQ8 phases.

## Admission And Dependencies

Consumes PIC. Admit the original MC146818A documentation and selected RTC
clock/divider/cascade wiring; host wall time is prohibited.

## Scope And Completion

Freeze register access, divider/reset, rate-select, update-ended, alarm,
periodic flag, Register-C clear, IRQ8/IRQ2 cascade, cancellation and reset.
Prove event order and no stale flag or IRQ after reset.

## Boundary

No host timestamp, oscillator measurement or profile callback may become guest
time. Battery persistence is a separate lifecycle decision.
