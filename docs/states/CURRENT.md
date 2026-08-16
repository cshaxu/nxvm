# Project Status

## Current Work

**T386 open; S1--S14 accepted; S15 active.** S15 reconciles the selected
Model-40 functional ledger against the current source and accepted S5--S14
evidence before another implementation slice. It allocates no device timing or
L3 result.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T386 S15. |
| Admission And Approval | Owner approved the continuing T386 selected-device functional and L3 program, including a capability-ledger-first sequence and implementation of the resulting gaps. This S is the bounded functional-ledger reconciliation required before the next repair; no exception. |
| Objective | Produce one current-source Model-40 selected-device functional matrix that classifies every selected platform, input, storage, display, interrupt/DMA, NMI/reset, memory/ROM and bus-facing requirement as complete, incomplete, externally constrained, or timing-only; assign each incomplete functional row to its earliest owner and next receiver. |
| Non-goals | No device implementation, no Core/VM behavior change, no new firmware or media import, no ROM path/hash/bytes, no physical-media inference, no board waits/service timing, no L3 or Windows claim, and no reopening historical task records. |
| Reference Baseline | T384 S1/S2 frozen Model-40 BOM and configuration; T385 CPU closure; T386 S5--S14 accepted evidence; current source at `fd58e422`; the DeskPro selected-device-functional and common L3 proposals. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md` with `docs/proposals/m5-80386-deskpro-386-l3-baseline.md`. |
| Files And ABI Surface | `docs/states/CURRENT.md`, one indexed T386 S15 evidence record, and `docs/etc/README.md` only. Source, build, public ABI and runnable artifact are read-only. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, T384 capability evidence, T386 proposal boundaries and source policy. The ledger must preserve Core generic-owner versus VM profile-composition ownership and distinguish functional closure from timing. |
| Verification | Reconcile every T384 selected requirement against source owner, current focused tests and accepted T386 evidence; execute source/static sweeps for Model-40 composition and selected core owners; run documentation governance; coordinator actual-change review. |
| Expected Markers | `M5:T386:S15:MODEL40-FUNCTIONAL-LEDGER:OK` and `M5:T386:S15:NEXT-RECEIVER:OK`. |
| Asset Needs | Primary documentation already cited by T384/S5/S6/S13 may be used only as transient evidence; no ROM, firmware, guest media, local path, hash, vendor text or third-party code is imported or committed. |
| Reporting Requirements | Record the full row-to-owner disposition in indexed evidence, identify the one next implementable functional receiver and all explicit stop/transfer rows, push the complete audit P, then perform coordinator actual-change review and governance P. |
| Stop Conditions | Stop and transfer a row if its selected observable is not source-defined, requires protected firmware, physical-media representation, board timing, or an owner decision beyond the frozen Model-40 configuration. Do not convert an unknown into a generic PC/AT behavior. |
| Exit Criteria | Every frozen selected component has a functional classification, owner, evidence/source basis and next receiver; no functional gap is hidden as timing; the next code S has one bounded owner/scope; evidence is indexed, governance passes, and coordinator review accepts the audit. |
| Original Owner Request | Continue under the governed queue plan to achieve DeskPro 386 Model 40 complete functionality and L3 timing; the user specifically required a device-completeness/support-surface ledger before timing closure and profile-specific behavior in VM versus generic hardware in Core. |
| Similar-Issue Sweep | Sweep the full T384 BOM, T386 S5--S14 evidence, Model-40 composition, selected device configuration APIs, focused current-gate tests, TODO and Queue receivers. For each row, classify current owner, missing functional observable, timing-only transfer or external constraint; report every unowned functional hit rather than narrowing the ledger to already-tested features. |
## Current Technical Baseline

- **Current developer artifact:** T386 S14 `vm-0-5-0386` /
  `build/output/nxvm_0_5_0386.exe`, SHA-256
  `254054B0700F9A9F286C13AD16D7214C7946C4BF4D916D75FD5257728B21E437`.
  Built from P1 `f8257a5d`; its 269-test serial current-gate and private
  Model-40 ROM-carrier proof are retained in
  [S14 evidence](../etc/evidence/t386-s14-model40-rom-layout.md).
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
| T386 S14 | Progress accepted: private Model-40 Rev-E ROM composition uses two 16 KiB odd/even chips, a mirrored 64 KiB system-ROM window and VM-owned A20 reset alias; Core remains unchanged. 269/269 serial current-gate plus composition, ownership and documentation gates pass. Public firmware, board behavior, timing and L3 remain transferred. [Evidence](../etc/evidence/t386-s14-model40-rom-layout.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen 5170 Model 339 deterministic-L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepted the then-current frozen Model-339 deterministic L3 contract. Its result is historical and is superseded for the current graph by T383. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |

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
