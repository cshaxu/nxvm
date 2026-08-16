# Project Status

## Current Work

**Active: M5 T382 S8 corrective.**

## M5 T382 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective |
| Admission And Approval | Owner-approved corrective reopening after closure review, 2026-08-15; S8 only. |
| Objective | Preserve 64-bit `memory_kib` exactly through catalog, Console and session option construction; reconcile current artifact baseline. |
| Non-goals | No memory schema/profile change or new allocation semantics. |
| Reference Baseline | T382 closure `75312b16` is rejected pending this correction. |
| Candidate Proposal | [M5 quality-boundary recovery](../proposals/m5-quality-boundary-recovery.md), S3 corrective continuation. |
| Files And ABI Surface | Console formatting, focused Console regression, current-gate registration, CURRENT/evidence/history. |
| Applicable Rules | Execution, architecture single-owner, coding bounded-failure, and documentation rules; no exception. |
| Verification | >32-bit KiB YAML-to-option round trip, Console lifecycle, current gate, artifact SHA-256, governance and final audit. |
| Expected Markers | `4294967296` remains exact at provider ingress. |
| Asset Needs | None. |
| Reporting Requirements | Complete executor P, then one-session coordinator actual-change review and governance P. |
| Stop Conditions | Any schema or VM allocation requirement beyond formatting/transport. |
| Exit Criteria | No narrowing conversion; CURRENT names final artifact and task-level closure only. |
| Original Owner Request | Correct rejected T382 closure without rewriting prior history. |
| Similar-Issue Sweep | Search formatting of `memory_bytes >> 10` and all CURRENT T382 artifact references. |

| Latest Closed Task | Compact result |
| --- | --- |
| T382 | Prior S1--S7 closure `75312b16` is under approved S8 corrective review; it is not accepted as final until this packet closes. |

## Current Technical Baseline

- **Current developer artifact:** T382 S8 candidate `vm-0-5-0382` /
  `build/output/nxvm_0_5_0382.exe`; final SHA-256 and current-gate proof await
  S8 evidence. The prior T382 closure is not accepted while this packet is active.
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
| T382 | Prior S1--S7 closure is under approved S8 corrective review; it is retained as a task-level record and not final until S8 accepts. [Prior audit](../etc/evidence/t382-task-closure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepts the frozen Model-339 deterministic L3 contract. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |
| T377 | Closed at `1cdd5110`: final audit rejects premature Model-339 L3 acceptance; its shared input/global-gate repair and independent re-audit successors are queued. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |
| T376 | Closed at `991060a4`: raw-IMG sidecar persistence, selected 8272A command fidelity and malformed-sidecar rejection are proven; physical duplicate-CHRN representation remains explicitly outside pure raw-IMG support. [Closure audit](../etc/evidence/t376-s6-task-closure-audit.md). |
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |

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
