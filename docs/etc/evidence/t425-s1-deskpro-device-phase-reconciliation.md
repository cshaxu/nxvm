# T425 S1: DeskPro Device-Phase Reconciliation

## Decision

T425 accepts only the existing deterministic logical device-phase contract for
receiver 2. It is `generic-at`, except that the selected Model-40 FDC
configuration/order remains `reference-derived` from T399 S3. No route is a
measured Compaq physical phase.

| Route | Sole owner and proven order | Tier | Physical transfer |
| --- | --- | --- | --- |
| FDC/DMA2/IRQ6 | Command -> execution -> DRQ eligibility -> DMA2 -> terminal IRQ6; DOR/reset cancellation deasserts DRQ/IRQ before rearm. | reference-derived configuration, generic-at mechanism | uPD765 clock, DACK, rotation, IRQ propagation. |
| HDC/IRQ14 | Command capture -> pending/BSY -> readiness advance -> DRQ/IRQ; sector continuation and SRST cancel pending state. | generic-at | Selected Compaq WD controller, media and physical latency. |
| KBC/IRQ1/IRQ12 | Serial queue/cadence -> output promotion -> source IRQ; port 60h acknowledges its origin and successor receives a fresh edge. | generic-at | 8042 serial/ACK and physical keyboard timing. |
| DMA/PIC | One arbitration callback grants DMA then advances PIT and refreshes PIC; transaction HOLD request/ack/release is reset-safe. | generic-at | HRQ/HLDA, DACK/AEN, INTA and propagation waveform. |
| D4 NMI | D4 latch -> port-61h enable/clear -> CPU pending-NMI consumer, with reset release. | original logical board fact | Electrical latch and NMI propagation duration. |

## Verification

Focused current-gate targets pass: Model-40 FDC, Core FDC, HDC, KBC serial
cadence, and PIC lifecycle. Existing T399/T407/T406/T419 evidence supplies the
route-specific cancellation and reset proof.

## Transfer

The remaining physical-observable device timing TODO remains unchanged: no
clock/rate, controller electrical phase, Compaq fixed-disk behavior, CECG
raster/monitor timing or firmware service behavior is claimed. This is not a
DeskPro physical/L3 acceptance.
