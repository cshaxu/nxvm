# Project Status

## Current Work

**Active: M5 T375 S21.**

| Task | Compact progress |
| --- | --- |
| T375 S20 | Accepted P1 `f0dee629`: FDC-owned 500-kbit/s DMA2 byte gates now use elapsed machine ticks; focused proof covers 128-tick cadence, final IRQ6 ordering and DOR cancellation. [Evidence](../etc/evidence/t375-s20-fdc-dma-cadence.md). |

## M5 T375 S21 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved T375 Model-339 L3 continuation. S19 names the aftermarket TEAC FD-235HF-A540 and admits its nominal 3-ms track-to-track value as a source-labelled input. |
| Objective | Gate FDC Seek and Recalibrate completion by the 24,000-tick per-track nominal movement interval, preserving the existing FDC IRQ6 and Sense-Interrupt owners. |
| Non-goals | No exact motor, rotational, index, controller-command, reset, waveform or factory-configuration claim; no raw-IMG work or DMA redesign. |
| Reference Baseline | S19 records TEAC FD-235HF-A540 nominal 3 ms per track and the Model-339 8-MHz tick conversion. S20 supplies timestamped FDC advancement. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | FDC state/phase owner, machine readiness caller, retained FDC smoke, evidence/index and Current only. |
| Applicable Rules | Execution lifecycle, architecture single FDC/DMA/IRQ owner, coding owner-local state, source policy and documentation rules. |
| Verification | Focused FDC smoke proves zero-distance and multi-track completion timing, cylinder visibility, IRQ6/Sense Interrupt, reset cancellation; retained DMA smoke and documentation governance pass. |
| Expected Markers | `M5:T375:S21:FDC-SEEK-CADENCE:OK` and retained FDC/DMA markers. |
| Asset Needs | No runtime asset; S19 primary-drive source only. |
| Reporting Requirements | Report implemented scope, source boundary, proof and transfers. |
| Stop Conditions | Stop if a per-drive movement state requires a second owner or an unsupported exact completion claim. |
| Exit Criteria | Seek/Recalibrate waits for the source-labelled nominal per-track deadline, owns no duplicate IRQ/DMA path, and cancellation prevents late completion. |
| Original Owner Request | Continue full L3 implementation using reference emulators only as secondary sources and keeping 1.44 MB as an aftermarket route. |
| Similar-Issue Sweep | Seek, recalibrate, Sense Interrupt, drive selection, DOR reset, finalize and FDC reset; transfer motor/rotation/timeout semantics. |

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
