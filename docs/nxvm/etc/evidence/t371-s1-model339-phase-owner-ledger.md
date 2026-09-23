# T371 S1: Model-339 Phase Owner And Source Ledger

## Admission result

The selected IBM PC/AT 5170 Model-339 has a deterministic logical phase graph,
but not yet a source-labelled physical phase scale.  Completed CPU retirement
advances project elapsed ticks; the one timeline then orders arbitration
(`DMA -> PIT -> PIC`), readiness (`FDC -> refresh -> HDC -> refresh -> RTC`)
and peripheral (`KBC -> VADP`) callbacks.  Transaction begin/commit/cancel and
logical 80286 DMA HOLD request/acknowledge/release are traceable within that
graph.  This is an admissible model-ownership foundation, not a numerical
wait-state, oscillator, pin-waveform or physical-cycle result.

The Model-339 primary baseline is IBM Type 3, 8 MHz 80286, 512 KB planar RAM,
CGA, no fixed disk and a compatible 1.44 MB field-upgrade drive.  IBM/Intel
authorities determine topology and controller/CPU protocol.  They do not, in
the evidence currently selected by this project, map every board signal and
controller clock to `core_machine` elapsed ticks.  Exact Intel CPU retirement
rows remain their own accepted input; reference emulators are not promoted to
IBM authority or numeric sources by this ledger.

## Complete phase-boundary matrix

| Phase boundary | Current sole owner and copied observer | Source status / S1 disposition |
| --- | --- | --- |
| Successful CPU retirement | executor publishes completed instruction cost; `machine.c` records `CPU_RETIRE` then advances the timeline. | T368 provides accepted successful-retirement rows. This does not turn all elapsed ticks into an 8 MHz physical clock; prefetch and external bus phase remain later S2. |
| CPU memory, ROM, CGA, ISA and port availability | shared transaction owner gates CPU/DMA transaction lifecycle. | IBM/80286 inputs do not select Model-339 READY/wait intervals. No value is admitted; S2 owns an explicit phase vocabulary and source-limit matrix. |
| DMA arbitration / HOLD | `dma.c` selects a request; `machine.c` arbitration owns logical HOLD request/ack/release and DMA transaction. | Dual-8237A/IBM topology and logical lifecycle are proven. HRQ/HLDA, DACK/AEN and maximum latency lack a project-domain conversion; S3 receives DMA/bus phase composition. |
| PIT/PIC and interrupt acknowledgement | arbitration advances PIT and refreshes PIC after DMA; PIC owns logical vector acknowledgement. | 8254/8259A semantics and topology are retained. Oscillator conversion, IRQ propagation and INTA waveform/spacing remain S3 physical-model transfers. |
| FDC/DMA2/IRQ6 readiness | FDC owns command/result/DRQ/IRQ; readiness advances and refreshes before a later arbitration opportunity. | uPD765 protocol is retained, but controller/drive timers and DRQ pace lack a selected mapping. S4 receives selected storage microstate and explicit non-admission proof. |
| RTC/CMOS/IRQ8 | RTC owns calendar/event state; readiness advances it after storage and CMOS owns NMI mask selection. | MC146818 semantics are retained. Oscillator/event phase and propagation have no project scalar; S4 receives device-clock boundary. |
| KBC/keyboard and VADP | one peripheral callback advances KBC then VADP; VADP publishes copied snapshots only. | Controller/display protocols are retained. 8042 serial/response timing and CGA dot/retrace/contention have no project mapping; S4 receives device-clock and copied-consumer boundary. |
| Planar parity/NMI | planar RAM path owns mismatch producer/latch; CMOS is mask-only and CPU is delivery consumer. | IBM source/latch topology is selected, not parity-detection/NMI propagation duration. S4 receives fault-phase boundary. |
| Reset, cancellation and trace | cold reset resets transaction, clock domains, controllers and timeline; finalization releases device sources; trace callback copies events. | Existing focused lifecycle tests establish deterministic reset/cancel order. Hardware reset settling and pin timing remain outside an admitted value. |

## Bounded T371 sequence

1. **S2:** phase vocabulary, CPU retirement/prefetch and memory/I/O source-limit ledger; no numeric wait selection without a qualified source.
2. **S3:** transaction, 80286 DMA/HOLD, PIT/PIC and interrupt-acknowledgement phase composition; reconcile any source-backed model boundary or retain it explicitly.
3. **S4:** selected device microstate/clock and reset composition for FDC, RTC, KBC, planar parity and CGA/VADP; do not convert an unqualified controller value.
4. **S5:** deterministic replay, copied trace, reset/cancellation and Model-339 phase-transfer audit; decide only whether T371 is ready for the subsequent 5170 L3 audit.

## Similar-route sweep and retained evidence

The S1 sweep covers the shared transaction and trace adapter, all three machine
timeline callbacks, every Model-339 clock-plan/configuration field, CPU
retirement publisher, DMA/PIT/PIC/FDC/RTC/KBC/VADP/NMI routes, cold reset and
finalization.  Focused T354 transaction/competition/lifecycle, T346 timeline,
T369 HOLD and accepted T370 device-owner smokes prove logical ordering and
source release.  They do not prove electrical or elapsed hardware duration.

The absence of a source-labelled project conversion means 86Box, MAME and
PCjs are not eligible to fill a numeric gap in S1.  A later S may use a named
same-profile reference only when the applicable authority gives a range or no
range and the packet explicitly qualifies the comparison; it cannot substitute
for a missing board/clock mapping.

**This S makes no 5170 Model-339 L3 or physical-cycle claim.**
