# Project Status

## Current Work

T381 S2 is active solely to correct the task's closure-state, history, and
archive consistency. It does not change the frozen YAML/Console behavior or
admit the next Queue candidate.

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective |
| Admission And Approval | The owner approved on 2026-08-15 the governance-audit corrections without changing `docs/rules`; the audit identifies T381 as the most recently closed numeric task whose retained `T381 S1` progress row contradicts its closure prose and machine-readable state. |
| Objective | Restore one truthful T381 closure: replace the stale open-subtask representation with a task-level closure record, retain a concise correction trail in T381 history, and remove no runtime evidence. |
| Non-goals | No source, build, artifact, profile, YAML, Console, Queue, DeskPro, 8088-XT, Windows, `docs/rules`, or new behavioral work. No historical commit is rewritten. |
| Reference Baseline | T381 S1 P1 `9b6a86c6`, its closure evidence and artifact record, P2 `e621a3c3`, the subsequent status-only P3--P6 commits, and the 2026-08-15 governance audit. |
| Candidate Proposal | [T381 retained proposal](../history/M5-T381-nxvm-console-redesign-proposal.md). |
| Files And ABI Surface | `docs/states/CURRENT.md` and `docs/history/M5-T381-nxvm-console-redesign.md` only; no runtime or ABI surface changes. |
| Applicable Rules | `DOCUMENT.md` authority boundaries and task-level closure retention; `EXECUTION.md` corrective-S, P lifecycle, actual-change review, reporting, and Documentation Governance Gate requirements. No architecture, coding, source, or runtime rule applies because no such surface changes. |
| Verification | Inspect P1--P6 actual diffs and retained T381 evidence; confirm the task-level state shape and T381 history; run the default Documentation Governance Gate and `git diff --check`; review the final actual diff. |
| Expected Markers | Documentation Governance Gate succeeds; no runtime marker or artifact hash changes because the runnable path is unchanged. |
| Asset Needs | None. |
| Reporting Requirements | Record the erroneous retained progress shape, the immutable P1--P6 disposition, the corrected task-level state, verification results, and the unchanged T381 artifact boundary. |
| Stop Conditions | Stop for owner direction if correcting the closure requires a rule change, runtime/build change, rewriting existing commits, or altering another task's historical record. |
| Exit Criteria | CURRENT has no active packet and one T381 task-level closure row; T381 history distinguishes S1 delivery from this S2 governance correction; no `T381 S1` open-progress row remains; the documentation gate passes. |
| Original Owner Request | Continue T381 S2 and later Td work to repair the governance audit findings, without modifying `docs/rules`. |
| Similar-Issue Sweep | Documentation-only recurrence: inspect CURRENT and T381 history for all retained T381 state references, and inspect the P1--P6 diffs. Other audited state/Queue findings are explicitly transferred to the separately admitted follow-on Td. |

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
| T381 S1 | Closed at `9b6a86c6`: frozen YAML profile selection, zero-session lifecycle and bounded floppy mutation are proven. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
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
