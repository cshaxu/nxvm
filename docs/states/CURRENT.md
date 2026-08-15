# Project Status

## Current Work

### S1: Frozen YAML session catalog and zero-session Console

| Field | Required record |
| --- | --- |
| Identifier Mode | New T381, S1; T380 is the latest closed numeric task. |
| Admission And Approval | Owner approved the Queue-head NXVM Console redesign on 2026-08-15 and separately approved the sole necessary core exception after the pre-implementation audit: `core/product/session` may represent zero sessions, create no implicit session, and close the final session. |
| Objective | Replace implicit default session creation with startup-frozen YAML session profiles beside NXVM, automatic startup `SESSION OPEN`, a true zero-session Console, and a selected-session-only command boundary. |
| Non-goals | No core machine/device/CPU/memory/timing change; no profile topology change; no DeskPro/XT work; no BIOS/ROM selector, guest-media import, disk-format work, persistence, GUI, public process CLI, or third-party YAML runtime. The core exception is limited to generic session-manager empty-state semantics. |
| Reference Baseline | T380 S2 `4bd5e2c4`, its recovery evidence, and the Queue-head Console redesign proposal. |
| Candidate Proposal | [NXVM Console redesign and frozen YAML session profiles](../proposals/m5-nxvm-console-redesign.md). |
| Files And ABI Surface | `src/vm/product/` Console/catalog owner, `src/vm/composition/session/` frozen-config construction/binding, `src/vm/main.c`, product/session tests and CMake registrations; the sole core surface may be `src/core/product/session/{manager.c,session_interface.h}` only as required for empty-manager semantics. No `src/core/machine/` change. |
| Applicable Rules | Architecture one-owner/one-production-path and composition direction; coding bounded C11 interfaces and no test-only API; source policy boundary; execution artifact/evidence/closure; documentation truthfulness. The core exception must remain generic and must neither parse YAML nor select VM profiles. |
| Verification | Product-facing tests prove retained banner/help then automatic chooser, filename-only sorted selection, no-file/cancel zero state, invalid-file containment, frozen snapshot behavior, YAML profile validation, selected-session command rejection, floppy-only stopped mutation, hard-disk immutability, and empty-manager lifecycle. Run focused tests, full current smokes, specialized gates, documentation governance, actual-diff audit, and emit T381 artifact. |
| Expected Markers | New T381 Console/YAML catalog and zero-session markers; existing T380 profile and generic-session markers remain green. |
| Asset Needs | Test-owned temporary text configuration and image-path strings only; no ROM, guest media, external source, local asset path, or protected binary is committed. |
| Reporting Requirements | Record YAML subset/validation and path-resolution contract, catalog freeze boundary, every retained/removed command disposition, core exception proof, focused and full-gate results, artifact SHA-256, and an explicit statement that no machine/device contract changed. |
| Stop Conditions | Stop for owner direction if a third-party parser/runtime, a public process CLI, external ROM/media, a profile-topology change, a core change beyond empty-manager semantics, or hard-disk hot-swap is needed. |
| Exit Criteria | The YAML-driven zero-session interaction and its command/media restrictions are proven through production paths; the core manager exception is bounded and tested; all required gates pass; T381 evidence/history/current status and artifact are truthful; the completed P is committed and pushed. |
| Original Owner Request | Owner requires YAML profiles displayed by file name, retained banner/help, automatic startup selection, zero sessions, global session commands plus HELP/EXIT only without selection, and no core changes except the later explicitly approved empty-session exception. |
| Similar-Issue Sweep | Audit every Console command dispatch and every core session-manager caller for implicit session construction, final-session prohibition, or selected-session dereference; fix within scope or record a bounded transfer. |

| Task | Compact progress |
| --- | --- |
| T380 S2 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |

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

- **Current developer artifact:** T381 S1 `vm-0-5-0381` /
  `build/output/nxvm_0_5_0381.exe`; its SHA-256 and current-gate proof are
  recorded at T381 closure.
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
| T380 S2 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
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
