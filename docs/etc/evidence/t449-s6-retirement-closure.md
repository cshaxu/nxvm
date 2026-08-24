# T449 S6: Retirement, Observation And Closure Audit

`M5:T449:S6:RETIREMENT-CLOSURE:OK`

## Sole Publication Ledger

| Fact | Owner and result |
| --- | --- |
| Successful CPU retirement | `machine.c:core_machine_publish_successful_retirement` is the one shared immediate/delayed seam; its two run-loop call sites precede the sole retired-time publication. |
| Elapsed guest time | `machine_scheduler.c:core_machine_publish_elapsed_ticks` is the only mutator of elapsed ticks and records CPU-retire or external-time trace before advancing the scheduler. |
| Transaction trace | Machine creation binds one transaction callback; transaction reset preserves that binding while cancelling/releasing dynamic state. The callback converts only typed transaction phases into the existing Core trace owner. |
| Retirement observation | CPU diagnostic capture creates one pending copied record; the successful-retirement seam publishes it once. Reset clears pending state; observers are installed only while stopped/paused and receive no mutable Core layout. |
| General observation | Stopped/paused diagnostics and machine observations are copied Core snapshots. They do not publish time, retire instructions or alter transaction ownership. |

## T449 Completion Reconciliation

| Proposal batch | Accepted evidence and disposition |
| --- | --- |
| S1 routes | `t449-s1-transaction-route-ledger.md`: frozen five-capability universe and direct-path residue. |
| S2 contract | `t449-s2-transaction-contract.md`: one validated copied selection contract replaces six raw fields. |
| S3 classes | `t449-s3-transaction-classification.md`: CPU/DMA/refresh success, cancel and reset paths are exhaustive; x87/device phases transfer explicitly. |
| S4 availability | `t449-s4-availability-contract.md`: one availability/BUSRDY/prefetch consumer and dynamic owner; physical waveforms remain L2. |
| S5 arbitration/memory | `t449-s5-arbitration-memory-contract.md`: one production arbitrator and memory classifier; direct DMA advance is unit-test-only. |
| S6 publication | This record: one retirement/time/trace/observation route, reset-safe copied observation, no added framework. |

The source sweep found no in-scope TODO receiver or unresolved duplicate route.
The retained L2 boundaries are explicit transfers to the queued controller/device
phase contract: physical READY/HOLD/DRAM waveforms, controller service duration,
selected parity detection/latch timing, and x87 ESC control semantics. They do
not leave an unclassified T449 mechanism.

Focused transaction/lifecycle, prefetch/locality, retirement-observation,
plan and Model-40 regressions pass, as do documentation governance and `git
diff --check`. S6 changes no tracked source/test path: the source/test delta is
`0/0`, because the audit found the required single mechanisms already present.
