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

The sole timing input is a copied `core_machine_rtc_timing_plan` at board
construction. It carries a finite UIP lead and update duration in already
selected RTC ticks, with explicit `L3_SOURCE` or `L2_RATIO` provenance. Core
never accepts a live setter, callback, host timestamp or second scheduler. A
profile may normalize an L2 ratio before construction, but that does not change
the plan's L2 provenance. An L3 declaration additionally requires an explicit
RTC rational clock and direct board timing values.

The work converges in four batches: source/corroboration ledger; RTC phase and
reset owner; board-plan provenance and integration; then current-gate, artifact
and closure audit. The generic MC146818 owner has 64 bytes only. IBM AT CMOS
byte 32h remains ordinary RTC RAM here and gains century meaning only in a
board/firmware map owner.

## Boundary

No host timestamp, oscillator measurement or profile callback may become guest
time. Battery persistence is a separate lifecycle decision.
