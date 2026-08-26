# T481 S2 CPU Function, Delivery And Timing Audit

`M5:T481:S2:CPU-OWNER:OK`

The S1 CPU denominator is retained: T343, with its T338--T341 source and
form records, is the sole function/state ledger.  This audit traces those
facts through the current one-owner execution path; it does not create a
second instruction table or alter a timing value.

| Family | Primary ledger and current Core path | Focused proof | Disposition |
| --- | --- | --- | --- |
| 8086 and 80186 real execution and profile forms | T343/T338; the one primary decoder, metadata classifier and `core_machine_cpu_execution_refresh()` own form selection and execution. | T343's retained low-profile corpus/form gates; `verify-t359-instruction-timing-inventory`. | Function is Manual-L3. Exact selected timing is L3 where T357/T359/T363 cite a table/formula; range/model and the retained compatibility result are explicit L2, never an invented manual value. |
| 80286 protected execution, delivery and timing contexts | T343/T339 plus T357 S6 and T360; the same refresh/metadata route preserves the 16-bit protected layouts rather than borrowing 80386 state. | T343 protected corpus; `verify-t360-timing-source-inventory`. | Function is Manual-L3. Appendix-B source-context rows selected by their ledger are L3; unresolved table/context and compatibility timing remain L2 receivers, not a false physical clock claim. |
| 80386 paging, VM86, task, debug and ordinary execution | T343/T340/T341 plus T357 S3/S7; the profile-local form classifier feeds the shared execution/retirement seam. | T343 80386 corpus; `core-machine-instruction-timing-ledger-smoke`; `verify-t359-instruction-timing-inventory`. | Function is Manual-L3. T357's documented selected no-wait forms/formulas are L3. Its visible one-tick unallocated result is L2 transfer policy, not an Intel timing assertion. |
| Successful retirement | T359 S7 and T363 S7; `core_machine_run()` calls `core_machine_cpu_timing_select()`, captures retirement eligibility, then uses the sole `core_machine_publish_elapsed_ticks()` publisher.  `cpu_timing.c` selects; it does not publish. | `verify-t435-s3-cpu-timing-seam`, `core-machine-instruction-timing-ledger-smoke`. | Source-backed form clocks are L3. Compatibility, range/model and nonphysical external-cycle portions retain their labelled L2 disposition. There is no handler-local elapsed-time writer. |
| Synchronous fault and exception delivery | T343 shared delivery and T361 S3; `ExecFinal()` owns fault-frame delivery and marks a delivered in-progress fault, while the run loop stops that round before retirement/tick publication. `ExecInt()` remains the separate NMI/PIC delivery route. | T361 S3 focused `#DE`, `#UD`, `#BR`, `#NM/#MF`, protected, paging, debug, VM86 and interrupt-entry regressions. | Architectural delivery is Manual-L3. It deliberately publishes no successful-retirement time for a faulting round; delivery ordering without a qualified timing formula is L1, not L2 or L3 timing. |
| CPU memory, I/O, INTA and DMA-shared transactions | T449 S3; CPU helpers and `dma.c` use the one transaction state, while `machine_scheduler.c` owns refresh occupancy.  External-cycle waits accumulate before the same retirement seam. | `core-machine-transaction-s2-smoke`, T449 retained transaction/competition/lifecycle regressions. | Commit/cancel and ordering are L3 logical contracts. A copied board/manual wait input may be L3 at its source; otherwise external-cycle and physical/prefetch/waveform timing remains explicit L2/L1 transfer, never a second transaction owner. |
| CPU-side x87 `WAIT`/`ESC` boundary | T343 external coprocessor row and the retained x87 TODO; CPU owns only the architectural interface and `#NM/#MF` delivery. | T361 S3 FPU-interface smoke. | Interface function is L3; numerical x87 execution and timing are outside this CPU census and remain the named L0 receiver. |

## Reconciliation And Transfers

The source sweep found one decoder/metadata classifier, one execution refresh
route, one successful-retirement selector/publisher seam, one exception-final
delivery owner and one transaction lifecycle.  The remaining lower-level
timing claims are already explicit rather than hidden in those owners:

- exact-form/context reconciliation remains in the retained T360/T388 CPU
  ledgers;
- physical bus waits, prefetch, HOLD/DMA and device service stay with the
  source-backed board/device timing receivers, not `cpu_timing.c`;
- numerical x87 remains the named `Broaden present x87` TODO receiver; and
- no profile may upgrade an L2 CPU timing result by calling the Core publisher
  directly.

`M5:T481:S2:TRANSACTIONS:OK`

`M5:T481:S2:CPU-AUDIT:OK`
