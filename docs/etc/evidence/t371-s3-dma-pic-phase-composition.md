# T371 S3: DMA, PIT/PIC And Logical Acknowledgement Composition

## Decision

The selected Model-339 has one deterministic arbitration owner.  At each due
tick, `machine.c` advances the DMA domain, conditionally performs the logical
80286 DMA HOLD request/acknowledge/transaction/release lifecycle, advances PIT,
then refreshes the dual PIC.  DMA owns request selection and transfer; the
transaction owner rejects CPU competition during an acknowledged DMA hold; PIT
owns counter/output; PIC owns source aggregation, priority, logical vector
selection, acknowledgement and EOI.  The copied trace records this order and
the competition smoke proves reset releases a pending hold.

Intel 8237A/8254/8259A and IBM PC/AT sources support those controller and
topology facts.  They do not map HRQ/HLDA, DACK/AEN, PIT clock, IRQ propagation
or INTA waveform/spacing into Model-339 project elapsed ticks.  Logical PIC
acknowledgement is therefore not labelled an INTA waveform.  No S3 duration,
wait state or runtime change is admitted.

| Boundary | Owner / retained proof | Phase result |
| --- | --- | --- |
| DMA request and transfer | `dma.c`, one transaction owner and T369 HOLD trace | Logical request/grant exclusion only; physical grant cycle remains unallocated. |
| HOLD lifecycle | transaction request/acknowledge/release, reset cancellation | Traceable lifecycle, not 80286 pin timing or maximum latency. |
| PIT then PIC | arbitration callback and focused arbitration/competition smokes | Deterministic order only; no oscillator or propagation conversion. |
| PIC acknowledgement/EOI | `pic.c` logical vector/ISR owner | Vector semantics retained; physical INTA phase remains unallocated. |
| Trace/reset | copied trace adapter and cold reset | Order and release are proven, not electrical settling. |

## S4 receiver

S4 receives selected device microstate and clock boundaries: FDC/drive, RTC,
KBC/keyboard, planar-parity NMI and CGA/VADP.  It must retain that controller
protocol or callback order does not authorize a device oscillator, mechanical
duration, serial delay, NMI propagation or display cadence value.

**This S makes no 5170 Model-339 L3 or physical-cycle claim.**
