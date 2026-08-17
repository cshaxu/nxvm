# Project Status

## M5 T391 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner: user. Approval: this thread's continuing instruction to audit and repair CPU/startup defects, prepare DeskPro timing/L3, and preserve Core/VM boundaries, 2026-08-17. Scope: inventory the existing post-C0 port-61h startup checkpoint and identify its earliest functional/CPU receiver. |
| Objective | Establish whether the accepted first post-C0 immediate port-61h read provides a finite startup semantic checkpoint from existing copied observation, and record the complete owner/caller/test boundary for the following C0A corpus admission. |
| Non-goals | No source, interface, runtime profile, ROM/media, asset, CPU timing row, physical-retirement, board-time, device-time, or L3 change; do not treat port 61h as a clock or infer firmware completeness. |
| Reference Baseline | T390 S33 post-C0 I/O evidence, T390 S34/S35 deterministic boundary, T390 S36 dependency requeue, T386 S29 functional closure, and the T391 startup-semantic proposal. |
| Candidate Proposal | [DeskPro 386 startup semantic readiness](../proposals/m5-deskpro-386-startup-semantic-readiness.md). |
| Files And ABI Surface | `docs/states/CURRENT.md`, `docs/history/M5-T391-deskpro-386-startup-semantic-readiness.md`, `docs/etc/README.md`, and indexed T391 S1 evidence only. No source or public/private interface surface changes. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/README.md`, and `CONTRIBUTING.md`; architecture/coding rules apply as unchanged-boundary constraints. |
| Verification | Existing `current.vm-model40-byob-retirement-capture` synthetic smoke; documentation governance; `git diff --check`; actual-diff review; source/owner/caller sweep. |
| Expected Markers | `M5:T391:S1:POST-C0-PORT61-INVENTORY:OK`. |
| Asset Needs | None. Existing retained aggregate evidence only; no asset, firmware, media, local path, hash, byte, trace, or external repository content is read, added, or changed. |
| Reporting Requirements | Report the finite checkpoint decision, current owner, absent/present defect decision, next corpus receiver, and retained boundaries; do not claim CPU qualification, board timing, or L3. |
| Stop Conditions | Stop and transfer if the checkpoint requires raw state, a VM-specific Core callback, a new port provider, external asset research, or cannot define a finite stage. |
| Exit Criteria | The port-61h checkpoint is explicitly accepted or rejected as C0A admission input; every owner/caller/test is recorded; a bounded next corpus receiver or earliest defect owner is named; focused and documentation checks pass. |
| Original Owner Request | Continue the global CPU instruction/timing audit, repair confirmed issues, prepare DeskPro board timing and L3 closure, and preserve Core/VM boundaries. |
| Similar-Issue Sweep | T390 S30/S32/S33/S34/S35/S36; Model-40 D4/port-61h owner and composition; capture synthetic/current registration; T386 functional closure; physical and board-time proposals. |
## Current Technical Baseline

- **Current developer artifact:** T390 S22 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `AF45AB7BF8D76CBFAD2EEE1C53BB9710CF408997CA9C78948196EE880AB140F8`.
  T386 closes selected-device functional completeness at S29; its retained
  [closure audit](../etc/evidence/t386-s29-functional-closure-audit.md) fixes
  HDC current-gate coverage and transfers board, firmware and physical work.
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
| T390 | Stopped by owner-approved requeue: C1 needs a bounded startup-semantic prerequisite while board timing requires physical qualification first. The Queue now places startup semantic readiness before renewed physical qualification. Model-40 remains deterministic; no Core/VM interface, board-time, physical-clock, or L3 claim. [Audit](../etc/evidence/t390-s36-stop-and-requeue-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
