# Project Status

## Current Work

**Active: M5 T376 S4.**

| Task | Compact progress |
| --- | --- |

## M5 T376 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuous M5 L3 implementation; accepted T376 S3 completes address-mark command routes and transfers only Scan. |
| Objective | Implement 8272A Scan Equal, Scan Low-or-Equal and Scan High-or-Equal through one existing FDC transfer/cadence/IRQ/reset path. |
| Non-goals | No new media format, filesystem API, test-only controller route, inferred address mark, second DMA engine, timing change, or final L3 decision. |
| Reference Baseline | T376 S2 raw-IMG mark lifecycle, T376 S3 deleted-data transfer/result path, T375 byte/seek gates, and existing FDC ST2 scan bits. |
| Candidate Proposal | [8272A raw-IMG sidecar fidelity closure](../proposals/m5-8272a-img-sidecar-fidelity-closure.md). |
| Files And ABI Surface | `core/machine/fdc.*`, existing FDC smoke/evidence/index/current/history; no host-side interface or core media ABI change. |
| Applicable Rules | One FDC transfer/DMA/IRQ owner, compare against guest-supplied bytes only, address marks through provider, existing cancellation and cadence invariant. |
| Verification | Focused FDC smoke proves Equal/LE/GE match and mismatch ST2 results, byte cadence, IRQ result phase and DOR cancellation; build and governance pass. |
| Expected Markers | `M5:T376:S3:8272A-DELETED-DATA:OK`; `M5:T376:S4:8272A-SCAN:OK`. |
| Asset Needs | Project-owned fixture media only; no firmware, guest media or external source import. |
| Reporting Requirements | Record opcode/compare/ST2 behavior, shared-owner proof, timing preservation, focused result and task-closure receiver. |
| Stop Conditions | Stop for owner direction if scan requires a separate transfer or a provider capability absent from the frozen ABI. |
| Exit Criteria | All Scan command families complete through current routes with verified result/cancel/timing behavior; only T376 task audit remains. |
| Original Owner Request | Continue through L3 before Windows; retain raw IMG compatibility and do not invent test-only APIs. |
| Similar-Issue Sweep | Inspect command decoding, command length, read/write DMA/non-DMA transfer, result/IRQ, reset/cancel and every existing ST2 consumer. |

## Current Technical Baseline

- **Current developer artifact:** T376 S4 `vm-0-5-0376` /
  `build/output/nxvm_0_5_0376.exe`; its SHA-256 and source commit are
  recorded by this S4's acceptance record.
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
| T376 S3 | Accepted P1 `707114ce`: 8272A Deleted-Data and Control-Mark behavior now uses the retained media, DMA, IRQ, reset and cadence owners; Scan remains next. [Evidence](../etc/evidence/t376-s3-8272a-deleted-data.md). |
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |
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
