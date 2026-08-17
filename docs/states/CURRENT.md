# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation - T390 S2. |
| Admission And Approval | Owner approved continuation on 2026-08-17. The owner also authorizes use of external owner-managed ROM and guest-media assets, provided no vendor ROM, guest media, hash catalogue, local path, or trace is committed. Scope is limited to a BYOB observation contract and current trace-gap decision; it does not authorize physical-profile enablement. |
| Objective | Freeze the bounded external Model-40 BYOB corpus/observation contract required to determine whether every successful 80386 retirement through a selected checkpoint is exact or stops before physical publication; record whether existing production trace can supply its required normalized observation. |
| Non-goals | No ROM/media import, catalogue, download mechanism, committed trace, emulator-as-oracle result, CPU timing-row assignment, Core or VM code change, physical retirement selection, board-clock ratio, device timing, firmware-completeness or Model-L3 claim. |
| Reference Baseline | T390 S1 inventory; T388 S9 physical rejection closure; selected DeskPro Model 40 profile/ROM BYOB boundary retained by T386. Existing current trace records retirement PC and elapsed ticks only. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | `docs/states/CURRENT.md`; `docs/history/M5-T390-80386-physical-retirement-qualification.md`; `docs/etc/evidence/t390-s2-model40-byob-observation-contract.md`; `docs/etc/README.md`. No source or ABI surface. |
| Applicable Rules | Execution lifecycle, actual-change review, documentation topology and source/research policy apply. The observation contract applies recorder-trace containment if a later S launches it. No rule exception is requested. |
| Verification | Review the current Model-40 BYOB composition and `core/machine` trace event/capacity owners; verify the contract neither stores protected asset identity nor turns external references into an oracle; run documentation-governance and whitespace checks. |
| Expected Markers | `M5:T390:S2:MODEL40-BYOB-OBSERVATION-CONTRACT:OK`; explicit `CURRENT-TRACE-INSUFFICIENT` decision; a named next receiver if a shared capture boundary is required. |
| Asset Needs | External owner-managed Model-40 ROM pair and bootable 1.2 MB DOS medium may be identified only outside the repository. This S does not launch them or retain their local paths, hashes, bytes, catalogue entries, or traces. |
| Reporting Requirements | Report contract decision, trace sufficiency, gate result, commit and pushed state; link durable evidence and name the next bounded receiver. |
| Stop Conditions | Stop and transfer if the required observation requires a new public/test-only control path, protected asset retention, unbounded recording, an undefined checkpoint, or a wider CPU mechanism than the contract can isolate. |
| Exit Criteria | Indexed evidence defines exact profile/configuration abstraction, normalized observation fields, reset/checkpoint/terminal states, hard recorder budgets and cleanup, and proves from source inspection whether existing trace can meet them. Task history and current state truthfully retain the S result. |
| Original Owner Request | Continue the queue after owner-approved requeue; qualify the shared 80386 retirement prerequisite before returning to DeskPro board timing, using external ROM/media only outside the repository. |
| Similar-Issue Sweep | Inspect all current CPU-retire trace emission and Model-40 BYOB construction sites, not just the first observed call, for enough context to distinguish exact from source-unallocated successful retirement. |
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
| T390 S1 | Accepted: source/classifier inventory separates exact 80386 rows from nonphysical successes and transfers a bounded Model-40 BYOB corpus observation contract. No physical profile, board clock or L3 claim. [Evidence](../etc/evidence/t390-s1-80386-physical-qualification-inventory.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
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
