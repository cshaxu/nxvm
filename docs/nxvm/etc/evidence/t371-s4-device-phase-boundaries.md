# T371 S4: Selected Device Phase Boundaries

## Decision

The Model-339 selected devices compose through the existing one timeline:
readiness advances FDC/refresh, HDC/refresh and RTC; peripheral advances KBC
then VADP; planar parity publishes through its RAM-latch-to-NMI route.  Reset
clears each transient owner and finalization releases device-owned sources.
The profile's deterministic ratios and RTC `50000` remain project scheduling
parameters, not physical conversions.

| Device boundary | Retained owner and phase fact | Physical boundary retained |
| --- | --- | --- |
| FDC/DMA2/IRQ6 | FDC command/DRQ/IRQ state advances then refreshes before later arbitration. | uPD765 clock, DRQ pace, seek/rotation/TEAC mechanics. |
| RTC/CMOS/IRQ8 | RTC event state advances after storage; CMOS bit 7 only masks NMI. | MC146818 oscillator/event propagation. |
| KBC/keyboard | One FIFO/IRQ owner advances before VADP; all Model-339 delay fields are zero. | 8042 clock, serial ACK and response/typematic time. |
| Planar parity/NMI | RAM mismatch latches through port 61h; mask and CPU delivery are separate. | Parity detection and NMI propagation. |
| CGA/VADP | CGA state/capture advances through VADP and copies snapshots to the host boundary. | Dot clock, retrace, contention and host cadence. |

No device-local callback or reset path supplies a second scheduler.  Existing
FDC, RTC, KBC, parity and CGA/VADP focused tests prove protocol, ordering and
lifecycle only.  S5 receives deterministic replay/trace/reset composition and
the task transfer audit.  No device duration is admitted, and this record makes
no 5170 Model-339 L3 claim.
