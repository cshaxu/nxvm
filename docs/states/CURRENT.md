# Project Status

## Current Work

**Active: M5 T375 S22.**

| Task | Compact progress |
| --- | --- |
| T375 S23 | Accepted P1 `83b05d66`: keyboard F3 now derives delay/repeat cadence from one nominal profile calibration, and reset/F5/F6 restore IBM's `2Ch` 500-ms/10-cps default. [Evidence](../etc/evidence/t375-s23-kbc-f3-cadence.md). |

## M5 T375 S24 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuing T375 L3 work; S19 establishes the selected 500-kbit/s byte interval and S20 leaves the equivalent non-DMA data-register path open. |
| Objective | Apply the existing 128-tick 500-kbit/s byte-availability gate to selected non-DMA read, write, and format data-register transfers. |
| Non-goals | No sector-search/index model, gap timing, spin-up completion time, 250-kbit/s timing, raw-IMG sidecar work, DMA redesign, host pacing, or final L3 claim. |
| Reference Baseline | DMA data bytes withdraw/reassert DRQ through the 128-tick FDC gate; non-DMA `3F5h` reads/writes presently transfer immediately while MSR continuously exposes RQM. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | FDC-local gate state/status/data-port logic, FDC smoke, evidence/index/current; no public or test-only API. |
| Applicable Rules | One FDC owner, one DMA owner, profile/source-local timing, reset cancellation, architecture/coding/source-policy/documentation rules. |
| Verification | FDC smoke proves first non-DMA byte is available, MSR withholds RQM at tick 127 after one byte, grants it at tick 128, and preserves result/IRQ6/reset plus DMA regressions. |
| Expected Markers | `M5:T375:S20:FDC-DMA-CADENCE:OK`; `M5:T375:S21:FDC-SEEK-CADENCE:OK`; `M5:T375:S24:FDC-NDMA-CADENCE:OK`. |
| Asset Needs | None; S19's named TEAC primary conversion supplies the 128-tick input. |
| Reporting Requirements | Report unified FDC ownership, non-DMA MSR/RQM behavior, retained mechanical and media-format limits, and no new API. |
| Stop Conditions | Stop if a status bit or gated port access is used to fabricate sector/rotation timing, or if DMA gets a second request path. |
| Exit Criteria | In selected 500-kbit/s non-DMA execution, every successful byte transfer is followed by a 128-tick RQM absence; due time restores RQM; cancellation clears it; existing DMA behavior remains proven. |
| Original Owner Request | Continue L3 precision toward all selected baseline machines before Windows, with primary sources first and no test-only input boundary. |
| Similar-Issue Sweep | Cover read, write, format, MSR, transfer completion/error, reset/finalize, DMA gate, CCR selection, and all `3F5h` execution paths. |

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
