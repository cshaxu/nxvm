# Project Status

## Current Work

## M5 T383 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner explicitly approved the first M5 Queue candidate on 2026-08-15. Scope is the independent current-source Model-339 re-audit only; no source repair, external asset import, or contract expansion is authorized. |
| Objective | Make one fresh, owner-visible ready/not-ready decision for the frozen Model 339 deterministic-L3 contract against the current runnable source graph, including every applicable T380 strict-start correction. |
| Non-goals | No runtime repair, new device capability, Model-339 variant, fixed-disk MFM/ATA route, physical/electrical timing claim, DeskPro/XT work, Windows claim, or ROM/media import. |
| Reference Baseline | T379 S1 accepted the then-current source at `160a34e1`; T380 S2 (`4bd5e2c4`) changed strict Model-339 startup. The runnable current artifact baseline is T382 S8 `2a5ac5a6`, `vm-0-5-0382`, SHA-256 `6507E88A85F9E85ED4EB12D1B0B376DED5894771D178E3D1B2B5B89202BC8D57`. |
| Candidate Proposal | [Model-339 current-source re-audit](../proposals/m5-5170-model339-current-source-l3-reaudit.md). |
| Files And ABI Surface | Audit-only: `docs/states/CURRENT.md`, `docs/history/M5-T383-model339-current-source-l3-reaudit.md`, `docs/etc/evidence/t383-s1-model339-current-source-reaudit.md`, and completion state. Product source and public ABI are read-only unless this packet is stopped and a separate repair task is admitted. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, Model-339 evidence contracts, and `docs/etc/operations/policy/source-policy.md`; no exception is requested. |
| Verification | Establish a clean current GCC build; run `run-current-smokes` and `verify-current-specialized-gates`; replay the Model-339 composition, clock-contract, CGA-topology, firmware/FDC-topology, FDC-DMA-boundary, session-profile, Console-lifecycle, native-keyboard and raw-IMG/FDC evidence rows; compare T379's accepted graph with `160a34e1..HEAD`; run documentation governance and inspect the actual source/test registrations and change diff. |
| Expected Markers | `M5:T366:S5:MODEL339-COMPOSITION:OK`, `M5:T380:S2:MODEL339-NO-XMS-PROBE:OK`, `M5:T380:S2:MODEL339-512K-FDC-START:OK`, `M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK`, and `M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK`. |
| Asset Needs | None. External ROMs and guest media remain owner-managed, outside Git, and are not an audit prerequisite. |
| Reporting Requirements | Record an explicit requirement-to-current-proof matrix, a T379-to-current change reconciliation, every residual receiver, and one ready/not-ready conclusion. Complete one implementation P, then switch to coordinator role for actual-change review and a separate truthful closure P if accepted. |
| Stop Conditions | Stop before any repair if a selected functional/timing proof is absent, the gate fails, a source defect is found, or the declared contract would need expansion. Transfer the earliest owner through Queue/TODO rather than repairing within T383. |
| Exit Criteria | Current-source matrix covers all selected rows and T380 changes; required gate and focused replay pass; exclusions and residual receivers are explicit; the independent ready/not-ready decision is evidence-backed; actual-change review and task-level closure audit are complete. |
| Original Owner Request | The owner requested the next action, then explicitly approved the proposed first Queue candidate. |
| Similar-Issue Sweep | Compare `160a34e1..HEAD` across Model-339 descriptor/composition, firmware, FDC, KBC/input, memory/A20, session/profile and their CTest registrations; identify whether any current-path change invalidates a T379 evidence row. |

## Current Technical Baseline

- **Current developer artifact:** T382 S8 `vm-0-5-0382` /
  `build/output/nxvm_0_5_0382.exe`, SHA-256
  `6507E88A85F9E85ED4EB12D1B0B376DED5894771D178E3D1B2B5B89202BC8D57`.
  Its 251-test current-gate and corrective reclosure proof are retained in
  [S8 evidence](../etc/evidence/t382-s8-console-memory-roundtrip.md) and the
  [corrective audit](../etc/evidence/t382-s8-reclosure-audit.md).
- **Model-339 readiness:** T379's deterministic-L3 decision remains historical
  evidence for its then-current source graph. T380 S2 subsequently changed the
  strict profile startup path, so the queued current-source re-audit owns the
  next ready/not-ready decision before DeskPro work begins.
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
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepted the then-current frozen Model-339 deterministic L3 contract. Its result is historical; the queued current-source re-audit owns the next readiness decision. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |
| T377 | Closed at `1cdd5110`: final audit rejects premature Model-339 L3 acceptance; its shared input/global-gate repair and independent re-audit successors are queued. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |
| T376 | Closed at `991060a4`: raw-IMG sidecar persistence, selected 8272A command fidelity and malformed-sidecar rejection are proven; physical duplicate-CHRN representation remains explicitly outside pure raw-IMG support. [Closure audit](../etc/evidence/t376-s6-task-closure-audit.md). |
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |

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
