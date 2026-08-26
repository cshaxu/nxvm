# T469 S3: Core PIT/RTC Deadline Selection

`M5:T469:S3:CORE-DEADLINE-SELECTION:OK`

## Implemented Finite Batch

Core now derives a deadline only from the first source-qualified batch frozen
by T469 S1:

- each PIT counter's next state/output boundary, including an enabled auxiliary
  PIT, using its existing mode, gate, reload and remaining-count owner state; and
- an L3-source RTC periodic or enabled update IRQ boundary, using the
  existing divider, rate and calendar owner state.

Each controller reports device ticks privately. The scheduler converts them
through the existing copied rational clock plan and selects the earliest Core
tick. `core_machine_advance_to_next_deadline()` advances that selected amount
through the existing timeline, so all intermediate callbacks keep their
existing ordering. VM supplies neither a tick count nor a deadline choice.

## Conservative Eligibility

The observation remains unavailable unless the copied plan labels the PIT and
RTC clock inputs as source rational clocks and RTC timing as `L3_SOURCE`.
It also remains unavailable while any of these unqualified state owners is
active: DMA service, KBC delayed/typematic delivery, FDC seek/byte/non-command
phase, or
HDC non-idle phase. This is a blocker, not an estimate and not a skipped
event. Default PC/AT therefore remains unavailable even after PIT programming.

## Verification

The PIT waveform smoke covers next-boundary calculation across modes 0--5,
gate pauses/triggers, pulse/reload boundaries and binary/BCD cases. The
Model-339 clock-contract smoke proves:

- the board-refresh PIT1 load boundary is first selected at Core tick 7; then,
  once PIT1 is gated off, the RTC's rational-clock phase produces the next
  periodic boundary at absolute Core tick 7813;
- PIT0 count loading produces the first Core boundary at tick 7;
- Core, not VM, advances to each selected boundary; and
- KBC, FDC, HDC and DMA activity plus default-PC/AT L2 timing make the copied
  deadline unavailable.

Existing PIT, RTC, Core-time, virtual-time and speed-policy smokes remain
required. No controller pointer, second scheduler, host-duration conversion or
new timing value was added.

## S4 Transfer

S4 must add immutable physical-timebase qualification around this dynamic Core
observation. It may expose host pacing only when the selected profile provides
that timebase and the current observation is valid; it must reject the default
PC/AT and every dynamically blocked state.
