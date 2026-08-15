# T372 S8: Model-339 Current-State Closure And Transfer Audit

## Decision

T372 closes as an audit-only task with **IBM PC/AT 5170 Model 339 L3 not
ready**. S1--S7 prove the selected profile, logical component ownership,
deterministic event/reset ordering, bounded CPU retirement ledger, and the
limits of available primary and secondary evidence. They do not prove that all
selected devices are functionally complete, nor do they select the
board-local waits, controller-service durations, event phases, or physical
measurement contracts required for a Model-339 L3 decision.

This conclusion neither rejects the future Model-339 baseline nor blocks it on
86Box, MAME, PCjs, or the retired Bochx/Bochs adapter. Those references remain
optional, bounded corroboration only. The ordered capability ledger and 5170
functional/timing candidates are now the receivers.

## Requirement And Transfer Matrix

| T372 audit requirement | S1--S7 result | Closure disposition and exact receiver |
| --- | --- | --- |
| Exact selected-machine identity and supported-surface boundary | Type 3, 8 MHz, 512 KB planar RAM, Rev.3 slot, CGA, AT keyboard, no fixed disk, planar parity, and aftermarket 1.44 MB floppy compatibility are established; DeskPro/XT and the product-wide surface are not yet reconciled with it. | The closed [baseline-machine and supported-device capability ledger](../../history/M5-T373-baseline-device-capability-ledger-proposal.md) freezes the three-machine bill of materials, support status, and every earliest functional/timing owner. |
| 80286 retirement and shared transaction foundation | Successful-retirement accounting and one CPU/DMA transaction lifecycle are accepted. Fetch/prefetch/external occupancy and board READY/wait phase remain unallocated. | The [5170 board and device phase-timing closure](../../proposals/m5-5170-board-phase-timing-closure.md) owns the profile-local phase contract; it must not reopen closed instruction semantics. |
| Memory/ROM, DMA, PIC, PIT, RTC, NMI, reset and bus-facing functional behavior | Selected topology and logical lifecycle/order are present, but no task has established a frozen selected-device completeness matrix that separates partial/empty state from timing-only work. | The closed [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md) owns register/state-machine, IRQ/DRQ, error/reset and consumer gaps; its timing handoff is mandatory. |
| 8042/AT keyboard and FDC/floppy function | Logical command/FIFO/IRQ/reset and DMA2/IRQ6 paths are present; controller/drive completeness and the selected aftermarket 1.44 MB configuration must be classified before duration is assigned. | The same closed [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md) owns functional classification and repair, with no factory-configuration claim. Its later service/phase questions transfer to the 5170 timing candidate. |
| CGA and planar-parity function | CGA topology, selected digital paths, copied-frame boundary, and planar-parity latch/mask/reset are present; remaining exposed CGA capability and any functional controller gap are not evidence of a completed selected device. | The closed [capability ledger](../../history/M5-T373-baseline-device-capability-ledger-proposal.md) classifies supported CGA surface; [5170 selected-device functional closure](../../history/M5-T374-5170-selected-device-functional-closure-proposal.md) closes selected behavior. Broader exposed-device equivalence remains the later [current-product device-capability closure](../../proposals/m5-current-product-device-l3-closure.md). |
| Board waits, arbitration, controller service and cross-device phase | IBM/Intel/controller material and qualified references provide no direct Model-339-to-project-domain scalar. Logical ordering is not a physical interval. | [5170 board and device phase-timing closure](../../proposals/m5-5170-board-phase-timing-closure.md) owns primary-source/probe constrained timing, range/reference-exhausted disposition, and no-synthetic-value policy. |
| Final Model-339 L3 decision | This task proves only not-ready and all residual categories; no implementation is absorbed. | [5170 final model-L3 audit](../../proposals/m5-5170-final-l3-audit.md) is the only later ready/not-ready decision after the ledger, functional, and timing candidates close. |
| IBM fixed disk | The selected baseline has no fixed disk; ATA/HDC is not IBM MFM/ST-506. | Retain [IBM 5170 MFM/ST-506 fixed-disk route](../../states/TODO.md#hardware-and-compatibility-debt) as excluded debt; it is not a prerequisite for the no-fixed-disk Model-339 baseline. |
| Optional historical Bochx/Bochs bridge | Manifest gate is valid but its adapter cannot execute against retired NXVM interfaces. It cannot observe platform or timing behavior. | Retain [bounded differential debugging](../../states/TODO.md#cpu-time-and-debugging-debt) as optional developer-tool migration; it is not an L3 receiver or blocker. |

## Evidence Review

S1 establishes the component-by-component not-ready matrix. S2--S4 reject
unqualified secondary-reference values and primary-fact overreach. S5 contains
external-asset readiness only; S6 rejects the unobservable external runtime;
S7 confines the historical bridge to an optional future CPU microprobe tool.
T366--T371 independently retain the same distinction between selected logical
ownership/order and unallocated functional completeness or board-phase timing.
No evidence conflicts with the ordered transfer matrix above.

## Closure Verification And Similar-Issue Sweep

The closure review reads all T372 S1--S7 records and T366, T369, T370, and
T371 closure transfers; it compares each residual with the current Queue and
proposal boundaries. The sweep finds no omitted selected Model-339 function,
no synthetic timing scalar, no false factory claim for the 1.44 MB field
upgrade, no ATA/MFM substitution, and no external asset, trace, path, hash, or
reference-output leakage. Documentation governance and `git diff --check` are
required before acceptance. T372 makes no executable, artifact, or L3-ready
claim.
