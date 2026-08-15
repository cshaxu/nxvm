# T375 S25: Model-339 Board-Phase Ledger

This is a source/status audit, not a timing implementation. It rechecks the
frozen Model-339 8-MHz Type-3 baseline against the current production graph
and the earlier T369/T371/T372 source limits.

| Boundary | Current owner/proof | Selected phase status | Receiver |
| --- | --- | --- | --- |
| 80286 retirement | executor and T368 ledger | successful-retirement rows only; source assumptions exclude prefetch, READY and HOLD | final bus model/audit |
| Fetch, RAM, ROM, I/O, ISA | transaction plus memory/port owners | no selected board-local READY/wait conversion | qualified measurement/reference or final audit |
| CGA aperture/contention | VADP/memory provider and S13 ratio | no CGA contention, snow or board-wait measurement | final audit |
| DMA grant | `dma.c`, arbitration and T348/T369 | logical request/HOLD/ack/release/one grant only; no HRQ/HLDA/DACK/AEN pin phase | qualified measurement/reference or final audit |
| PIT/PIC/INTA | arbitration/PIC and T350/T349 | logical source/priority/vector acknowledgement only; no propagation or INTA spacing | final audit |
| FDC DMA2/IRQ6 | FDC and S20/S21/S24 | byte/seek gates are source-backed; controller-to-grant and mechanical/index boundaries remain | raw-IMG sidecar/final audit |
| Reset/cancel | reset/timeline/transaction owners | deterministic release only; no physical settle or pin timing | final audit |

At each arbitration tick, `machine.c` advances DMA, PIT and PIC. A DMA
request records logical HOLD request, acknowledge, one DMA advance and release
through the same transaction owner. The later readiness and peripheral ticks
advance device owners. This is deterministic visibility, not a statement that
one project tick equals an 80286 bus phase or a pin waveform.

The sweep covered transaction phases, DMA advance, arbitration/readiness, PIC
acknowledgement, memory/port availability, clock plans and focused tests. No
second transaction, DMA or PIC owner exists. T372 still admits no numeric
reference value: no fully matching recorded 86Box/MAME/PCjs observation
qualifies a board scalar, and MAME leaves the ISA clock undetermined.

The remaining rows cannot be repaired by arbitrary waits. Any numeric selection
requires a primary range/absence plus a revision-pinned matching observation or
measurement contract. This ledger blocks a premature 5170 L3 claim.

`M5:T375:S25:MODEL339-BUS-LEDGER:OK`
