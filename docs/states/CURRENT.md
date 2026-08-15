# Project Status

## Current Work

**Active: M5 T375 S22.**

| Task | Compact progress |
| --- | --- |
| T375 S22 | Accepted P1 `a1c67ed3`: Model-339 now binds IBM's nominal 500-ms/10-cps typematic values with retained +/-20-percent interpretation; command-response timing remains an upper-bound transfer. [Evidence](../etc/evidence/t375-s22-model339-typematic.md). |

## M5 T375 S23 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuing T375 L3 work; primary IBM PC/AT Technical Reference F3 definition exposes the default-parameter and runtime-decoding gap. |
| Objective | Make valid AT-keyboard F3 parameters derive the active typematic delay/repeat cadence from the Model-339 nominal calibration, and restore the IBM 500-ms/10-cps default parameter. |
| Non-goals | No exact keyboard oscillator claim, ACK completion claim, physical serial transmission model, host delay, generic AT profile timing change, or final L3 claim. |
| Reference Baseline | S22 profile binds 4,000,000/800,000 nominal ticks, but KBC stores default `20h` and F3 only stores its byte without changing active timing. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | KBC default/parameter decoder and internal state, controller smoke, Model-339 contract smoke, evidence/index/current. No new test-only or cross-module API. |
| Applicable Rules | Execution lifecycle, one KBC owner, profile-local calibration, source-policy, architecture and coding rules, documentation rules. |
| Verification | Controller smoke proves reset/F5/F6 default `2Ch` and F3 rate/delay change; Model-339 smoke proves 4,000,000/800,000 default cadence and a selected F3 derivation; documentation governance passes. |
| Expected Markers | `M5:T227:S3:KBC-CONTROLLER:OK`; `M5:T375:S22:MODEL339-TYPEMATIC:OK`; `M5:T375:S23:KBC-F3-CADENCE:OK`. |
| Asset Needs | None; primary reference is the already cited IBM Technical Reference. |
| Reporting Requirements | Report the prior default/decode defect, the nominal-derived arithmetic, and the retained +/-20-percent and physical-clock boundaries. |
| Stop Conditions | Stop if F3 ranges are represented as exact physical keyboard periods or if an invalid parameter protocol policy is invented without a source. |
| Exit Criteria | Default state is `2Ch`; every valid F3 value updates active cadence from one preserved profile calibration; reset/default commands restore it; focused proofs pass. |
| Original Owner Request | Continue L3 precision work and keep direct input as a native production boundary, not a test-only API. |
| Similar-Issue Sweep | Search all typematic default, F3 parameter, reset/default, profile-calibration, timing setter, and proof sites; fix or classify every production hit. |

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
