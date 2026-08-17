# Project Status

## Current Work

## M5 T389 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T389 S1 is accepted retained progress, and S2 performs the owner-approved stop-and-requeue disposition. |
| Admission And Approval | Owner approval, 2026-08-17: stop and requeue T389, add the shared 80386 physical-retirement qualification prerequisite before a new DeskPro board task. Repository/Git authority remains granted. |
| Objective | Close T389 truthfully as a non-implementation stop caused by its invalid physical-time prerequisite, archive its proposal, and establish the approved ordered replacement candidates without claiming board timing or L3 completion. |
| Non-goals | No CPU, Core, VM, device, firmware, media, artifact, ABI, profile, timing or L3 behavior change; no rewriting S1 history; no assignment of a physical clock scalar. |
| Reference Baseline | T389 S1 reconciliation, T388 S9 safety closure, T359/T360/T363 timing ledgers, T386/T387 transfers, current Queue, ROADMAP, EXECUTION, DOCUMENT and source policy. |
| Candidate Proposal | [DeskPro 386 board-level timing closure](../proposals/m5-deskpro-386-board-level-timing-closure.md). |
| Files And ABI Surface | `CURRENT.md`, `QUEUE.md`, T389 history/proposal archive, replacement proposals and supporting index only. No source, build, public/runtime ABI, configuration, asset or media surface. |
| Applicable Rules | EXECUTION governs continuation, task stop, history and identifiers; DOCUMENT governs archival/link/state topology; ROADMAP fixes the CPU-before-board dependency; source policy retains no firmware/media/reference import. |
| Verification | Inspect S1 and referenced source/timing evidence; prove the replacement prerequisite owns the exact gap without absorbing board/device work; run `git diff --check` and documentation governance. |
| Expected Markers | `M5:T389:S2:STOP-AND-REQUEUE:OK`. |
| Asset Needs | None. No ROM, firmware, guest-media, Intel PDF download, or secondary-reference execution. |
| Reporting Requirements | State that T389 is stopped rather than board-complete; name the replacement order and preserve all existing physical/nonphysical boundaries. |
| Stop Conditions | Stop if the replacement qualification would infer cycles, become a generic CPU-completeness claim, or require moving device/firmware timing into its scope. Record an explicit receiver instead. |
| Exit Criteria | T389 is task-level closed as an owner-approved non-implementation stop; its retained proposal/history links remain valid; Queue begins with bounded 80386 physical-retirement qualification followed by a fresh DeskPro board candidate. |
| Original Owner Request | Continue under the Queue toward complete DeskPro 386 function and L3 timing; owner explicitly approves stopping and reordering T389 around the shared prerequisite. |
| Similar-Issue Sweep | Inspect current Queue/DeskPro/CPU timing proposals and T359/T360/T363/T388/T389 evidence for an existing candidate that already owns the shared physical-retirement qualification; classify duplicates or gaps. |
## Current Technical Baseline

- **Current developer artifact:** T386 S28 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `80EC4B58630B163ECAFED6FE5B97F3B69D5E22D87928811FE66A8D5274E64EDD`.
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
| T389 S1 | Accepted: post-T388 Model-40 board reconciliation retains neutral deterministic clock domains and transfers shared 80386 physical-retirement qualification before board-clock implementation. No physical-clock or L3 claim. [Evidence](../etc/evidence/t389-s1-model40-board-timing-reconciliation.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
