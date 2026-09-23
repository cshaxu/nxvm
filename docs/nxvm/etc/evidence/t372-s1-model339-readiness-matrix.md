# T372 S1: Model-339 L3 Readiness Matrix

## Preliminary decision: not ready

The selected IBM 5170 Model-339 configuration, CPU retirement ledger, logical
bus/device graph, reset/cancellation lifecycle and copied trace are evidenced.
They prove deterministic model behavior, not the board-specific physical
mapping required by this task's source-labelled Model-L3 definition.  The
baseline is therefore **not ready** for a 5170 L3 closure claim.

| Component | Evidence-backed result | L3 readiness | Exact missing boundary / receiver |
| --- | --- | --- | --- |
| Profile/topology | Model 339 Type 3, 8 MHz 80286, 512 KB planar RAM, CGA, no disk, selected FDC/DMA2/IRQ6 and parity route. | Partial | ROM/board topology does not select timing conversion; phase implementation receiver. |
| 80286 retirement | Accepted successful-retirement source ledger and one publisher. | Partial | Prefix/ESC residuals, prefetch, fetch and external bus occupancy are not board phases; phase implementation receiver. |
| Memory/I/O availability | One transaction owner and checked memory/port routing. | Not ready | READY/waits for RAM, ROM, CGA, ISA and ports lack a Model-339 source/qualified observation. |
| DMA/PIT/PIC | Logical HOLD lifecycle and deterministic `DMA -> PIT -> PIC` order. | Not ready | HRQ/HLDA/DACK/AEN, PIT clock conversion, IRQ/INTA spacing and waveform are unallocated. |
| FDC/RTC/KBC | One command/state/IRQ/reset owner per controller. | Not ready | uPD765/drive, MC146818 and 8042 clock mappings and service delays are absent. |
| Parity/CGA | Selected parity latch/mask and CGA state/copy route. | Not ready | NMI propagation, CGA dot/retrace/contention and display cadence are absent. |
| Reset/trace/replay | Copied trace, cancellation and deterministic reset-order smokes. | Partial | Proves ordering only; it is not a physical probe or settle-time measurement. |

No residual is hidden by the audit: all are the same source-qualified phase
implementation gaps passed from T371.  A later T372 S must decide, per gap,
whether primary board documentation, a qualified same-profile reference model
under the user-approved policy, or an explicit reference-exhausted transfer
can supply a source-labelled model boundary.  The audit cannot close Model 339
L3 until that work completes.
