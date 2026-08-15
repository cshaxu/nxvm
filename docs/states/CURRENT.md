# Project Status

## Current Work

**Active: M5 T375 S22.**

| Task | Compact progress |
| --- | --- |
| T375 S25 | Accepted P1 `ec96f316`: source/status ledger confirms one logical transaction/DMA/PIC graph and names every unqualified Model-339 READY/HOLD/INTA/ISA/CGA boundary; no arbitrary wait is admitted. [Evidence](../etc/evidence/t375-s25-model339-bus-phase-ledger.md). |

## M5 T375 S26 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuing Model-339 L3 work; the accepted queue order makes this T375 completion audit the prerequisite for raw-IMG sidecar and final 5170 audit candidates. |
| Objective | Close the T375 board/device phase-timing task only by reconciling every required Model-339 route to accepted evidence or an explicit queued receiver, then record a truthful task-level transfer audit. |
| Non-goals | No source behavior change, guessed board wait, physical-cycle claim, raw-IMG sidecar implementation, final Model-339 L3 decision, firmware/media import, or repair of a later task. |
| Reference Baseline | T375 S1--S25, especially selected FDC byte/seek gates, keyboard cadence, CGA clock/status evidence, virtual time, and the S25 bus-phase ledger; Queue orders raw-IMG sidecar then final 5170 audit. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Evidence/index/current/history only; no production API, profile, ROM, media, or test behavior may change. |
| Applicable Rules | Execution task-level closure, documentation authority boundaries, source-policy evidence distinctions, and no unqualified board-timing conversion. |
| Verification | Reconcile proposal required-scope rows against actual evidence and current source-owner ledger; verify every unresolved row has the earliest valid Queue/TODO receiver; run documentation governance. |
| Expected Markers | `M5:T375:S25:MODEL339-BUS-LEDGER:OK`; `M5:T375:S26:TASK-CLOSURE-TRANSFER:OK`. |
| Asset Needs | None; this audit neither imports nor packages firmware, media, external source, or local assets. |
| Reporting Requirements | State the accepted T375 facts, each residual receiver, task-closure decision, verification, and that Model-339 L3 remains unaccepted. |
| Stop Conditions | Stop if a residual has no valid queue/debt receiver, an accepted fact cannot be tied to evidence, or closure would imply final L3 acceptance. |
| Exit Criteria | A task-level audit maps all required proposal routes to evidence/receiver, keeps raw-IMG and final audit ordering intact, indexes the audit, passes governance, and retains one task-level T375 closure row. |
| Original Owner Request | Fully converge L3 before Windows, using 86Box/MAME/PCjs only as bounded secondary references after primary sources; do not claim unresolved hardware behavior as complete. |
| Similar-Issue Sweep | Reconcile every proposal required-scope class: CPU and bus, PIT/PIC/RTC, KBC, FDC/drive/media, CGA, NMI/reset, and cross-device ordering; examine Queue/TODO for a missing or incorrectly ordered receiver. |

## Current Technical Baseline

- **Current developer artifact:** T369 S4 `vm-0-5-0369` /
  `build/output/nxvm_0_5_0369.exe`; its SHA-256 and source commit are
  recorded in the T369 S4 closure audit.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T375 S18 | Accepted P1 `8a4487e1`: explicit virtual-time publication emits a distinct core trace event; persistent replay and Model-339 physical timing remain open. [Evidence](../etc/evidence/t375-s18-external-time-trace.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine capability ledger and receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: selected Model-339 device-service ownership is reconciled; unavailable duration transfers to phase refinement. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: accepted 5170 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility; board/device timing remains open. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: source-labelled 80286 successful-retirement timing closes; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: VM owns concrete machine selection and CPU/timing contract binding; no CPU-timing, bus, device or L3 receiver closes. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
