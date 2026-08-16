# Project Status

## Current Work

**Active: M5 T382 S2.**

## M5 T382 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: repository user. Approval: 2026-08-15 request to execute the queue's first code-quality-governance task in one-session dual-role mode. Approved scope: T382 S2 from the accepted quality-boundary proposal. No exception permits restoring retired Console commands or changing session/profile semantics. |
| Objective | Remove the unreachable legacy `DEVICE` implementation and every now-unreferenced Console-provider route while retaining the frozen YAML session/profile and stopped-session floppy surface. |
| Non-goals | Do not restore or alias `DEVICE`, `SET`, `MODE`, or `TEST`; do not change core session behavior, YAML schema, hardware/media behavior, or S3--S7 domains. |
| Reference Baseline | T382 S1 P1 `8ab02ff5`, acceptance P2 `6adc1b3b`, and the retained S2 proposal contract. |
| Candidate Proposal | [M5 quality-boundary recovery](../proposals/m5-quality-boundary-recovery.md), S2. |
| Files And ABI Surface | Expected: `src/vm/product/console.c`, `console_machine_provider.h`, the VM Console adapter, and focused Console/product smoke sources. General VM session APIs remain outside the removal unless proven Console-only. |
| Applicable Rules | `docs/rules/EXECUTION.md` lifecycle, P, similar-issue and build-hygiene rules; `docs/rules/ARCHITECTURE.md` single route/owner invariant; `docs/rules/CODING.md` cohesive owner and no duplicate abstraction rules; `docs/rules/DOCUMENT.md` evidence ownership. No exception requested. |
| Verification | Console command/help/source sweep; retained Console lifecycle and input-failure smokes; focused session-catalog proof; fresh full current and specialized gates; documentation governance, diff check, artifact rebuild, and SHA-256. |
| Expected Markers | Retained Console commands have one reachable dispatch route; no production `DEVICE`/`SET`/`MODE`/`TEST` legacy command route or dead provider helper remains; focused and current gates pass. |
| Asset Needs | No new asset. Existing local smoke media remain optional and must retain diagnostic behavior. |
| Reporting Requirements | Executor confirms the route inventory and reports focused proof before one complete pushed P. Coordinator then switches roles, inspects the pushed actual changes and sweep, and accepts/rejects through the next P. |
| Stop Conditions | Stop for a retained non-Console caller of a proposed removal, any compatibility requirement to restore a retired command, a required API/ABI change outside the adapter, or an interaction with S3--S7; record the hit and request direction. |
| Exit Criteria | The unreachable block and unreferenced Console-only routes are removed; all dispatch/help/failure/lifecycle callers are classified; focused proof, current gates, artifact/evidence, review, and governance closure are complete. |
| Original Owner Request | Execute the first Queue code-quality-governance task under the latest governance rules in one-session dual-role mode. |
| Similar-Issue Sweep | Search tracked production Console/product/VM adapter code, focused tests, help text, and interactive lifecycle callers for retired command names and Console-only provider hooks; remove or classify each production hit and preserve only separately owned general session APIs. |

## Current Technical Baseline

- **Current developer artifact:** T382 S1 `vm-0-5-0382` /
  `build/output/nxvm_0_5_0382.exe`; its SHA-256 and current-gate proof are
  recorded in the active S1 evidence.
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
| T382 S1 | Accepted: P1 `8ab02ff5` adds the owned Windows aggregate deadline, Job Object cleanup, abnormal-child proof, and current artifact `vm-0-5-0382`; [evidence](../etc/evidence/t382-s1-current-gate-aggregate.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepts the frozen Model-339 deterministic L3 contract. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |
| T377 | Closed at `1cdd5110`: final audit rejects premature Model-339 L3 acceptance; its shared input/global-gate repair and independent re-audit successors are queued. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |
| T376 | Closed at `991060a4`: raw-IMG sidecar persistence, selected 8272A command fidelity and malformed-sidecar rejection are proven; physical duplicate-CHRN representation remains explicitly outside pure raw-IMG support. [Closure audit](../etc/evidence/t376-s6-task-closure-audit.md). |
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |

## Recent Governance

- **M5 Td S97 P1:** added explicit single-/separate-session review navigation
  without changing the lifecycle requirements.
- **M5 Td S96 P1:** compressed duplicate Role cycle prose into references to
  its retained authorities without changing lifecycle requirements.
- **M5 Td S95 P1:** removed duplicate M5 technical narrative from Queue while
  retaining all shared-context, candidate, and required proposal-index links.
- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
