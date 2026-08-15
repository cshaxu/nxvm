# Project Status

## Current Work

**Active: M5 T375 S22.**

| Task | Compact progress |
| --- | --- |
| T375 S21 | Accepted P1 `212306df`: FDC seek/recalibrate now retain per-drive position and delay IRQ6/Sense-Interrupt completion by 24,000 ticks per track. [Evidence](../etc/evidence/t375-s21-fdc-seek-cadence.md). |

## M5 T375 S22 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved T375 continuation; IBM PC/AT keyboard primary material provides default typematic values and +/-20 percent tolerance. |
| Objective | Bind Model-339 only to IBM nominal 500-ms/10-cps typematic values and prove the resulting ticks lie inside the documented tolerance. |
| Non-goals | No exact ACK latency, no generic PC/AT change, no host input delay, no 8042 clock claim, no final L3 claim. |
| Reference Baseline | Model-339 descriptor currently leaves typematic zero; core KBC already owns delayed repeat. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Model-339 descriptor, profile smoke, evidence/index/current. |
| Applicable Rules | Execution lifecycle, profile-local ownership, source-policy and documentation rules. |
| Verification | Model-339 profile smoke proves 4,000,000 and 800,000 nominal ticks and their +/-20 percent bounds; documentation governance passes. |
| Expected Markers | `M5:T375:S22:MODEL339-TYPEMATIC:OK`. |
| Asset Needs | None. |
| Reporting Requirements | Report nominal/range distinction and retained ACK boundary. |
| Stop Conditions | Stop if a range is represented as an exact physical completion. |
| Exit Criteria | Model-339 selects IBM nominal typematic only, preserves other profiles and zero ACK delay. |
| Original Owner Request | Continue L3 accuracy with primary sources first and bounded reference use only. |
| Similar-Issue Sweep | Default PC/AT, Model-339 contract copy, KBC initial/repeat/response fields and profile tests. |

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
