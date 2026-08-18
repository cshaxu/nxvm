# T397 S2: DeskPro Physical-Device Timing Task-Closure Audit

`M5:T397:S2:TASK-CLOSURE-AUDIT:OK`

## Proposal Reconciliation

| Required scope | Current evidence | Closure and L3 precondition |
| --- | --- | --- |
| 1.2 MB FDC/DMA2/IRQ6, including RAW-IMG versus physical media | T386 functional route, T397 ledger, 13 focused regressions and full current gate retain command/DRQ/IRQ/reset behavior | Logical route is accepted. Physical command-to-DRQ/IRQ availability requires the new precise physical-observable timing TODO; L3 must report it as not-ready, not model-derived. |
| Compaq 40 MB controller/IRQ14 | T386 normal PIO geometry route and existing physical-media TODO | Logical route is accepted. ECC, format, sector identity and drive-service timing remain the existing precise physical-media receiver; L3 remains not-ready for them. |
| CECG board/firmware timing | T386 CECG functional closure, selected VADP/copied presentation and existing CECG TODO | Digital selected behavior is accepted. Raster/monitor/sync/ISA/firmware-visible availability remain the CECG TODO; L3 remains not-ready for them. |
| KBC, reset and NMI paths with source-backed timing requirement | T397 Core owner/reset mapping, D4 functional regressions and D3PE topology facts | Functional reset/latch behavior is accepted. Physical KBC/failsafe/IOCHK propagation requires the new physical-observable timing TODO; L3 remains not-ready. |
| DMA/BWAIT availability | T396 Core transaction/DMA ledger and D3PE signal diagrams | Logical arbitration is accepted. No signal-to-retirement conversion exists; the new physical-observable timing TODO is the sole receiver. |
| Primary source/probe boundary and existing ownership | D3PE source boundary, T387 reference rejection, T396/T397 non-admission rules | Complete. No generic emulator, host clock, asset import, Core/VM interface or scheduler was introduced. |
| Verification | 13 focused S1 CTests and S2 `cmake --build --preset current-gates-gcc` all pass; documentation governance passes | Complete on unchanged runnable behavior. |

## Closure Decision

T397 closes through explicit retained receivers. It proves the selected
functional owners and reset/order behavior, but no selected route meets the
three-part physical-observable contract. The final DeskPro L3 audit is now
eligible only to make an owner-visible ready/not-ready decision; it cannot
upgrade any transferred fact into readiness or implement a repair.