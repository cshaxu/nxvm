# Project Status

## Current Work

## M5 T384 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner-approved enduring objective directs the single-role lifecycle to execute the ordered DeskPro sequence after the completed 5170 re-audit. Scope is the first Queue candidate only: profile provenance and capability-gap audit, with no runtime repair, firmware import, or L3 decision. |
| Objective | Lock the original 1986 Compaq DeskPro 386 Model 40/DeskPro 386/16 from Compaq primary documentation, and produce its complete selected-capability and owner-gap handoff for the later CPU, functional and board-timing tasks. |
| Non-goals | No CPU/device implementation, DeskPro profile source, board-timing allocation, generic clone inference, Windows claim, ROM/media import, vendor-byte record, or ready/not-ready L3 decision. |
| Reference Baseline | T383 accepts only the 5170. T373 records DeskPro as an empty machine composition with source-unknown display, controller and route fields. The shared DeskPro closure context and this candidate define the five-stage sequence. |
| Candidate Proposal | [DeskPro Model 40 profile and capability-gap audit](../proposals/m5-deskpro-386-model40-profile-capability-audit.md). |
| Files And ABI Surface | Audit-only: `docs/states/CURRENT.md`, `docs/history/M5-T384-deskpro-model40-profile-capability-audit.md`, `docs/etc/evidence/t384-s1-deskpro-model40-profile-capability-audit.md`, supporting index and closure state. Product source, ABI, ROM/media and assets are read-only. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/etc/operations/policy/source-policy.md`, T373 ledger, and the DeskPro closure context; no exception is requested. |
| Verification | Read and cite Compaq's September 1986 First Edition Technical Reference Guide as the primary source; distinguish its facts from PCjs hosting and secondary cross-checks; reconcile every selected component against current source/test inventory and T373; require one owner and later receiver per gap; run documentation governance, link validation, and actual-change review. |
| Expected Markers | `M5:T384:S1:DESKPRO-MODEL40-PROFILE-AUDIT:OK` and an evidence matrix with one explicit classification, owner and receiver for CPU, memory/ROM, display, input, FDC, fixed disk, PIC/PIT/DMA/RTC/NMI/reset and corpus. |
| Asset Needs | No asset. The BIOS slot remains an external owner-managed research input; this task records neither a vendor byte, local path, hash catalogue nor runtime dependency. |
| Reporting Requirements | Record source provenance and exact unselected/unknown fields, BOM, current capability disposition, earliest functional owner and later timing receiver. Complete one audit P, then switch to coordinator review before closure. |
| Stop Conditions | Stop if Compaq primary material cannot select a field, or if a proposed selection would rely on clone, emulator or generic PC/AT inference. Retain it as source-unknown with a bounded evidence/probe receiver; do not implement a substitute. |
| Exit Criteria | The Model 40 identity and every selected component has a primary-source or explicitly unknown disposition; the capability-gap matrix makes no inherited generic-device claim; CPU, functional and board-timing handoffs are complete; evidence, governance verification and closure audit are reviewable. |
| Original Owner Request | The active owner objective directs governance-compliant, single-person dual-role execution of the Queue plan through complete DeskPro 386 functionality and L3 timing. |
| Similar-Issue Sweep | Reconcile all existing DeskPro/Compaq references, generic 80386/PC-AT descriptor paths, display/storage/input/controller assumptions, and T373 unknown fields so no existing default profile is accidentally promoted to Model 40 evidence. |

## Current Technical Baseline

- **Current developer artifact:** T382 S8 `vm-0-5-0382` /
  `build/output/nxvm_0_5_0382.exe`, SHA-256
  `6507E88A85F9E85ED4EB12D1B0B376DED5894771D178E3D1B2B5B89202BC8D57`.
  Its 251-test current-gate and corrective reclosure proof are retained in
  [S8 evidence](../etc/evidence/t382-s8-console-memory-roundtrip.md) and the
  [corrective audit](../etc/evidence/t382-s8-reclosure-audit.md).
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. This is only the
  selected Model 339 configuration and preserves every documented physical,
  fixed-disk, generic-PC/AT and later-machine boundary.
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
| T383 | Closed: independent current-source replay accepts the frozen 5170 Model 339 deterministic-L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepted the then-current frozen Model-339 deterministic L3 contract. Its result is historical and is superseded for the current graph by T383. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |
| T377 | Closed at `1cdd5110`: final audit rejects premature Model-339 L3 acceptance; its shared input/global-gate repair and independent re-audit successors are queued. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |
| T376 | Closed at `991060a4`: raw-IMG sidecar persistence, selected 8272A command fidelity and malformed-sidecar rejection are proven; physical duplicate-CHRN representation remains explicitly outside pure raw-IMG support. [Closure audit](../etc/evidence/t376-s6-task-closure-audit.md). |

## Recent Governance

- **M5 Td S101 P1:** added a compact execution-entry table without creating
  new paths, and made coordinator semantic review explicit after structural
  documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339
  current-source re-audit before DeskPro, reconciled T380 S2 history, and made
  host/workspace path detection reject the external-assets form while retaining
  guest DOS-path examples.
- **M5 Td S99 P1:** compacted documentation gates, closure evidence, artifact
  identity, build hygiene, and recorder containment within Execution; design
  authorities remain unchanged.
- **M5 Td S98 P1:** made coordinator-review inputs, exit criteria, and the
  executor-report boundary explicit in Role cycle.
- **M5 Td S97 P1:** added explicit single-/separate-session review navigation
  without changing the lifecycle requirements.
- **M5 Td S96 P1:** compressed duplicate Role cycle prose into references to
  its retained authorities without changing lifecycle requirements.
- **M5 Td S95 P1:** removed duplicate M5 technical narrative from Queue while
  retaining all shared-context, candidate, and required proposal-index links.
- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
