# Project Status

## Current Work

**No active implementation packet.** T380 closes the owner-approved retained
Console profile-selection insertion. DeskPro 386, 8088-XT and Windows 3.1
remain unstarted.

| Task | Compact progress |
| --- | --- |
| T380 | Closed at `987bdd96`: `SESSION OPEN` now discovers and selects the default PC/AT or strict Model-339 profile; named selection, state visibility and Model-339 constraint rejection are proven. [Evidence](../etc/evidence/t380-s1-session-profile-selection.md). |
| T377 | Closed at `1cdd5110`: Model-339 is not L3-ready until the native input/global-gate repair and independent re-audit complete. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |
| T378 | Closed at `42f0402a`: native input proof, virtual-time dependency direction and affected gates are repaired; no Model-339 L3 decision is made. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `9099d11d`: independent full-gate and focused re-audit accepts the frozen Model-339 deterministic L3 contract. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |

<!-- T379 packet retired at task closure.

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner's current goal limits work to full 5170 L3 before any DeskPro 386, 8088-XT or Windows work; T378 closed at `42f0402a`, making this Queue-head re-audit eligible. |
| Objective | Independently repeat the frozen IBM PC/AT 5170 Model 339/Type 3 final L3 audit against the repaired production-input and current-gate baseline. |
| Non-goals | No implementation repair, DeskPro/XT/Windows work, generic AT claim, MFM/ATA substitution, physical waveform claim, ROM/media import or silent exclusion of selected capability. |
| Reference Baseline | T377 negative audit, T378 repair evidence, Model-339 re-audit proposal, T373--T376 selected ledger/functional/timing/raw-IMG evidence and current profile tests. |
| Candidate Proposal | [IBM PC/AT 5170 Model-339 L3 re-audit](../history/M5-T379-model339-l3-reaudit-proposal.md). |
| Files And ABI Surface | T379 history, audit evidence/index/current/queue status only; no source/runtime ABI change. |
| Applicable Rules | Independent complete requirement/component matrix, native production-input and reset/cancellation replay, source-backed or explicitly bounded deterministic L3 contracts, no guessed scalar, current-gate truthfulness and visible ready/not-ready result. |
| Verification | Re-run focused Model-339/FDC/input proof and full current gate; inspect all selected rows and T378 changes; run documentation governance and actual-diff review. |
| Expected Markers | Model-339 composition/clock/CGA/FDC, native-input, raw-IMG/8272A and current-gate markers; no new behavior marker without a separately admitted repair. |
| Asset Needs | None; external ROM/media remain outside Git and are not required for this audit. |
| Reporting Requirements | Record fresh matrix, T378 regression disposition, factory-versus-aftermarket distinction, physical boundary/receiver disposition, full-gate result and final 5170 L3 ready/not-ready decision. |
| Stop Conditions | Stop for owner direction if a selected functional/timing defect requires repair, a configuration change is needed, a timing claim lacks qualified evidence, or an external asset is required. |
| Exit Criteria | T379 closes only after every selected Model-339 row is independently reconciled, current gate passes, and its ready/not-ready conclusion leaves no hidden receiver. |
| Original Owner Request | Fully achieve 5170 L3, then stop before DeskPro 386, 8088-XT and Windows 3.1; do not replace native behavior with a test-only API. |
| Similar-Issue Sweep | Recheck every selected CPU, bus, input, DMA/PIC/PIT/RTC, FDC/floppy, CGA, NMI/reset/cancellation and T378's shared production boundaries. |
-->

## Current Technical Baseline

- **Current developer artifact:** T380 S1 `vm-0-5-0380` /
  `build/output/nxvm_0_5_0380.exe`; its SHA-256 and source commit are
  recorded by the [T380 S1 evidence](../etc/evidence/t380-s1-session-profile-selection.md).
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
| T380 S1 | Accepted P2 `987bdd96`: retained Console profile discovery/selection is usable, selected-state proof is explicit, and Model-339 identity constraints remain enforced. [Evidence](../etc/evidence/t380-s1-session-profile-selection.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepts the frozen Model-339 deterministic L3 contract. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |
| T377 | Closed at `1cdd5110`: final audit rejects premature Model-339 L3 acceptance; its shared input/global-gate repair and independent re-audit successors are queued. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |
| T376 | Closed at `991060a4`: raw-IMG sidecar persistence, selected 8272A command fidelity and malformed-sidecar rejection are proven; physical duplicate-CHRN representation remains explicitly outside pure raw-IMG support. [Closure audit](../etc/evidence/t376-s6-task-closure-audit.md). |
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine capability ledger and receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
