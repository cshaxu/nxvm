# T474 S4 Controller Deadline Composition

`M5:T474:S4:CONTROLLER-DEADLINE-COMPOSITION:OK`

## Single Core Route

`core_machine_capture_time_observation_private` is the sole deadline
composer. It first rejects an absent copied plan or any active blocking owner,
then considers only source-qualified PIT counters (including the optional
auxiliary PIT) and an L3-source RTC IRQ. Each private device duration is
converted through its copied rational clock; Core keeps the earliest value and
publishes it only as a copied observation. `core_machine_advance_to_next_deadline`
then submits that one delta to the existing Core time writer, which advances the
existing timeline and every intermediate callback in order.

No VM code selects a device or supplies a duration. VM only reads the copied
observation and requests the existing Core operation after its Standard/Turbo
policy has decided that running may continue.

## Full Controller Disposition

| Owner | Current disposition |
| --- | --- |
| PIT / auxiliary PIT | Eligible only with `SOURCE_RATIONAL_CLOCK`; its owner computes the next output boundary and Core converts it through the copied PIT clock. |
| RTC | Eligible only with `SOURCE_RATIONAL_CLOCK`, configured RTC state and `L3_SOURCE` RTC timing; its owner computes the next IRQ boundary. |
| DMA | Blocks while either controller has a pending request. Its source-qualified board service declaration is not a future observable-device deadline model. |
| KBC | Blocks while typematic, response or serial delivery remains pending. |
| 8272A FDC | Blocks while seek, DMA/NDMA byte gate or a non-command phase is active. |
| ATA/HDC | Blocks while its phase is non-idle. |
| PIC | No future deadline: it arbitrates already-pending IRQ visibility in the existing same-tick Core route. |
| VADP | No guest-wakeup deadline: presentation is a copied-snapshot consumer and its current timing has no admitted source-qualified future IRQ/event model. |
| Unqualified profile plan | No deadline, even if a controller is programmed; the L2 fallback never becomes a numeric estimate. |

This is the complete admitted controller universe from T474 S1 A5--A7. The
source-qualified Model-339 PIT/RTC plan proves the positive route; default PC/AT
proves the unqualified negative route. The explicit blocker checks prove no
pending controller is skipped during fast advance.

## Proof And Simplicity

- Built and ran `vm-model-339-clock-contract-smoke`,
  `core-machine-pit-waveform-smoke`, `core-machine-time-smoke`, and
  `vm-session-speed-policy-smoke`.
- The Model-339 smoke proves the PIT/RTC earliest-boundary order, Core-only
  advance, Standard pacing, Turbo no-wait, and each DMA/KBC/FDC/HDC blocker.
- The PIT waveform smoke covers modes 0--5, gates, reloads, pulses and BCD;
  Core-time and speed-policy smokes preserve the single observation and
  Standard/Turbo consumer boundary.
- No source or test code changed: the prior T469 implementation already meets
  this exact generic batch. A second scheduler, deadline callback, estimate or
  controller setter would duplicate the retained Core route and was not added.
