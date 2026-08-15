# Project Status

## Current Work

**Active: M5 T376 S6.**

| Task | Compact progress |
| --- | --- |

## M5 T376 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuous M5 L3 implementation; the owner's prior pure-IMG decision resolves S5's duplicate-CHRN stop by excluding non-representable physical-ID selection, while S5 identifies one in-scope malformed-sidecar regression gap. |
| Objective | Prove malformed raw-IMG sidecar rejection through the existing VM FDD mount boundary, record the pure-raw physical-ID exclusion, and supply T376's remaining closure evidence. |
| Non-goals | No new image format, CHRN/flux/CRC representation, production media API, host input route, FDC command change, physical timing claim, or Model-339 L3 decision. |
| Reference Baseline | T376 S2 strict sidecar parser/lifecycle, T376 S5 reconciliation, owner-selected pure IMG boundary, and existing `vm-media-provider-smoke`. |
| Candidate Proposal | [8272A raw-IMG sidecar fidelity closure](../proposals/m5-8272a-img-sidecar-fidelity-closure.md). |
| Files And ABI Surface | Existing VM media-provider smoke, T376 evidence/history/index/current/proposal/queue closure records; no production ABI change. |
| Applicable Rules | One production FDD mount owner, no test-only path, raw bytes remain independently usable, malformed metadata rejects before publication, and physical-ID capability remains visibly excluded. |
| Verification | Focused VM media-provider smoke proves malformed sidecar rejects with candidate FDD absent and prior mount unchanged; replay FDC smoke, build, governance gate, proposal/queue/history links and clean worktree. |
| Expected Markers | `M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK`; `M5:T376:S3:8272A-DELETED-DATA:OK`; `M5:T376:S4:8272A-SCAN:OK`; `M5:T376:S6:MALFORMED-SIDECAR:OK`. |
| Asset Needs | Project-owned generated fixture only; no external ROM, guest media or third-party source. |
| Reporting Requirements | Record malformed-input form, pre-publication failure proof, pure-IMG duplicate-CHRN exclusion, complete T376 matrix, task closure and next Queue receiver. |
| Stop Conditions | Stop for owner direction if rejection requires a new sidecar syntax, a test-only production entry point, or any non-raw physical-sector representation. |
| Exit Criteria | Malformed-sidecar rejection is directly proven, pure-IMG scope is accurately bounded, and T376 can close with its proposal retained in history and final Model-339 audit next. |
| Original Owner Request | Continue through L3 before Windows; retain pure raw IMG compatibility, no test-only media API, and no false physical-media or L3 claim. |
| Similar-Issue Sweep | Inspect all sidecar load failures, FDD insertion publication boundaries, existing generated-image cleanup, T376 requirements/evidence/index/history/proposal/queue links and every duplicate-CHRN claim. |

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
| T376 S5 | Accepted P1 `62e29252`: task audit proves raw-sidecar, Deleted-Data and Scan rows, but stops on duplicate-CHRN selection not representable by the approved pure raw-IMG model; owner decision required before final 5170 audit. [Audit](../etc/evidence/t376-s5-task-closure-audit.md). |
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
