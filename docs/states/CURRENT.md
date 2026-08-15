# Project Status

## Current Work

**Active: M5 T377 S1.**

| Task | Compact progress |
| --- | --- |

## M5 T377 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner-approved continuous M5 L3 implementation; T376 closed at `991060a4`, making this ordered audit candidate eligible. |
| Objective | Independently reconcile the frozen IBM PC/AT 5170 Model 339/Type 3 baseline against its capability, functional, timing and raw-IMG evidence and make an evidence-backed ready/not-ready L3 decision. |
| Non-goals | No repair implementation, generic AT/clone conclusion, MFM/ATA substitution, physical waveform claim, Windows claim, or silent exclusion of an unresolved selected capability. |
| Reference Baseline | T373 capability ledger, T374 selected-device functional closure, T375 board/device timing transfer audit, T376 raw-IMG closure, Model 339 final-audit proposal and current roadmap/queue. |
| Candidate Proposal | [IBM PC/AT 5170 final model-L3 audit](../proposals/m5-5170-final-l3-audit.md). |
| Files And ABI Surface | T377 history, audit evidence/index/current/queue status only; no source or runtime ABI change. |
| Applicable Rules | Audit-only scope, independent requirement-to-evidence and machine/component matrices, explicit source/external-model qualification, no guessed timing scalar, visible ready/not-ready decision and receiver for each residual. |
| Verification | Re-read selected baseline records and current source/test ownership, replay applicable current and focused gates, inspect all T377 changes and run documentation governance. |
| Expected Markers | T376 S2/S3/S4/S6 markers plus established Model-339 focused ownership/cadence markers; no new behavior marker unless an audit harness is required. |
| Asset Needs | No external asset or firmware; local artifacts/evidence only, with ROM and guest media remaining external. |
| Reporting Requirements | Record complete requirement matrix, evidence quality, raw-aftermarket distinction, cross-device/reset replay disposition, final ready/not-ready decision and exact repair/debt receiver. |
| Stop Conditions | Stop for owner direction if audit requires repair work, external asset import, an unselected configuration decision, or a timing claim unsupported by qualified evidence. |
| Exit Criteria | T377 closes only after a documented Model-339 L3 ready/not-ready decision maps every selected capability and residual to evidence or receiver; a negative decision must not be softened into acceptance. |
| Original Owner Request | Continue through L3 before Windows; Model 339 first, use external emulators only as bounded secondary evidence, and never claim a finished L3 baseline without proof. |
| Similar-Issue Sweep | Inspect every selected CPU, memory/ROM, bus, DMA/PIC/PIT/RTC, input, FDC/floppy, CGA, NMI/reset/cancellation and cross-device ordering row plus every prior Model-339 readiness claim. |

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
| T376 | Closed at `991060a4`: raw-IMG sidecar persistence, selected 8272A command fidelity and malformed-sidecar rejection are proven; physical duplicate-CHRN representation remains explicitly outside pure raw-IMG support. [Closure audit](../etc/evidence/t376-s6-task-closure-audit.md). |
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine capability ledger and receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: selected Model-339 device-service ownership is reconciled; unavailable duration transfers to phase refinement. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: accepted 5170 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility; board/device timing remains open. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: source-labelled 80286 successful-retirement timing closes; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
