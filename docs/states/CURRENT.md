# Project Status

## Current Work

**T386 open; S1--S18 accepted; S19 active.** S19 corrects the S18 technical-
baseline provenance record only: the accepted 0389 artifact and 273/273 serial
gate replace stale S17/272 wording. It adds no runtime behavior, profile,
artifact or L3 claim.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T386 S19. |
| Admission And Approval | The owner authorized continued single-role governed DeskPro work. Coordinator discovery found S18's accepted CURRENT technical-baseline prose still said S17/272 while naming S18 artifact 0389; correct the truthful record only, with no exception. |
| Objective | Correct S18 current-baseline provenance to name its S18 worktree, 273/273 serial current-gate and floppy device-profile proof. |
| Non-goals | No source, build, artifact, test, profile, YAML, firmware, media, timing, device functionality or L3 change. |
| Reference Baseline | Accepted T386 S18 P1 48820dd3 and P2 fa7b6e96; indexed S18 evidence; artifact hash 29A174DBC889A1220B0F4BD17805AAA78B326F01B31E5D68D5688F6B5DEC09E4. |
| Candidate Proposal | docs/proposals/m5-deskpro-386-model40-device-functional-closure.md. |
| Files And ABI Surface | docs/states/CURRENT.md and indexed S18 evidence only; no code or ABI surface. |
| Applicable Rules | EXECUTION and DOCUMENT rules: CURRENT is sole current technical baseline and must truthfully agree with accepted artifact/evidence. |
| Verification | Inspect accepted P1/P2, artifact SHA-256, current-gate evidence and exact diff; run documentation governance and coordinator actual-change review. |
| Expected Markers | M5:T386:S19:BASELINE-PROVENANCE:OK. |
| Asset Needs | None. |
| Reporting Requirements | Record the correction and acceptance in S18 evidence; do not claim a new artifact or runtime verification. |
| Stop Conditions | Stop if correction requires source/build behavior changes or contradicts accepted evidence. |
| Exit Criteria | CURRENT names S18/0389/273 accurately, evidence records the corrective provenance result, docs gate and actual-diff review pass. |
| Original Owner Request | Continue governed DeskPro functionality/L3 work with truthful evidence and architecture boundaries. |
| Similar-Issue Sweep | Inspect all S18 CURRENT artifact/proof wording and evidence references; correct only the stale provenance statement. |

## Current Technical Baseline

- **Current developer artifact:** T386 S18 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `29A174DBC889A1220B0F4BD17805AAA78B326F01B31E5D68D5688F6B5DEC09E4`.
  Built from the S18 worktree after its 273-test serial current-gate; the
  floppy device-profile proof is retained in
  [S18 evidence](../etc/evidence/t386-s18-model40-floppy-device-profile.md).
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
| T386 S18 | Accepted: reusable VM device-profile floppy geometries select Model-40 1.2 MB raw media while Core retains neutral media/FDC/DMA mechanics; default and Model-339 retain 1.44 MB. Public variants, physical media, timing and L3 remain open. [Evidence](../etc/evidence/t386-s18-model40-floppy-device-profile.md). |
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
