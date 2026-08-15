# T371 S5: Model-339 Phase Replay And Transfer Audit

## Replay coverage

The retained focused corpus proves the assembled logical graph: T346 timeline,
arbitration, readiness/storage and input/display markers establish due-tick
order; T354 transaction and lifecycle markers establish one copied transaction
trace plus cancellation/reset; T369 proves logical 80286 DMA HOLD ordering.
Together they cover CPU retirement, transaction begin/commit, DMA/PIT/PIC,
FDC/RTC/KBC/VADP callbacks and reset replacement of pending events.  They are
deterministic replay evidence, not physical measurement.

## T371 audit matrix

| T371 surface | Proven logical result | Residual receiver |
| --- | --- | --- |
| CPU/transaction | One retirement publisher and transaction lifecycle. | Prefetch, READY, memory/I/O waits and bus phase remain reference-exhausted. |
| DMA/PIT/PIC | `DMA -> PIT -> PIC`, logical HOLD and vector acknowledgement. | HRQ/HLDA/DACK/AEN/INTA waveform and latency remain unallocated. |
| Selected devices | One callback/state/reset owner per FDC, RTC, KBC, planar parity and VADP. | Controller clocks, drive mechanics, serial exchange, NMI propagation and CGA cadence/contention remain unallocated. |
| Trace/reset | Copied trace and cancellation/reset ordering. | No electrical settling or physical timing assertion follows. |

T371 therefore closes only as a source-labelled logical phase foundation.  Its
physical, board-specific and reference-exhausted items transfer intact to the
following IBM 5170 Model-L3 closure audit, which must make the explicit
ready/not-ready decision.  **T371 does not establish that the 5170 Model-339
has reached L3.**
