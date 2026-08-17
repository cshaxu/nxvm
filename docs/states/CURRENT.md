# Project Status

## Current Work

**Active.** T390 S5 is admitted as its next bounded continuation.

## M5 T390 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved continuation of the Queue-governed T390 work on 2026-08-17 and previously authorized uncommitted owner-managed ROM/media use. S4 is accepted; this S admits only the normal 1.2 MB media bridge and bounded boot-corpus observation needed by S2. |
| Objective | Extend the contained Model-40 BYOB capture route to insert an owner-managed bootable 1.2 MB floppy through the normal session media path, then observe the selected corpus from cold reset to its first boot-sector control-transfer checkpoint or truthfully stop at the first terminal boundary. |
| Non-goals | No ROM/media commit, local path/hash/content/provenance in Git, default runtime dependency, test-only production ingress, ROM-decode claim, physical retirement selection, board/device timing, generic boot-completeness, guessed timing or Model-L3 claim. |
| Reference Baseline | Accepted T390 S2 BYOB observation contract, T390 S3 copied Core observer, T390 S4 preboot matrix and `6f5550f9`; the selected Model-40 remains physically nonqualified under T388. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | The existing external capture helper may accept the optional owner-managed floppy input and use existing `vm_session_config.fdd_image`; bounded helper output/termination and project-owned focused coverage may change. No Core, public ABI, profile wiring, firmware, guest data or default product path changes are admitted unless a separately approved packet revision is required. |
| Applicable Rules | Source/research policy governs owner-managed firmware/media; architecture requires the existing session/media boundary; execution requires one complete pushed P, actual-change review and evidence; coding rules govern changed helper/tests; documentation topology governs Current/history/evidence. No exception is requested. |
| Verification | Prove helper argument validation and normal Model-40 1.2 MB startup insertion with project-owned focused coverage; perform one contained external run using owner-managed inputs without retaining identities; enforce S2's process/output/wall/no-progress/retirement limits; record only normalized checkpoint/terminal form matrix; run focused proofs, source/caller sweep, documentation governance and full current gates. |
| Expected Markers | Existing `M5:T390:S3:RETIREMENT-OBSERVATION:OK`; a new S5 normalized boot-corpus checkpoint or truthful terminal/transfer marker, never a physical or L3 marker. |
| Asset Needs | Read an owner-managed valid Model-40 ROM pair and bootable 1.2 MB floppy only outside the repository. The asset names, locations, hashes, bytes, provenance and raw output remain uncommitted and unrecorded. |
| Reporting Requirements | Report only abstract corpus availability, checkpoint/terminal reason, normalized disposition summary, source scope, verification, evidence and pushed state. Explicitly state that physical selection, board timing and L3 remain open. |
| Stop Conditions | Stop and transfer if normal media insertion is unavailable, containment cannot be met, the checkpoint cannot be recognized without wider observer data, a successful form is unallocated, an exact Intel row is absent, or physical/board/device work would be needed. Do not enlarge the observer or enable a partial physical path. |
| Exit Criteria | The normal-media receiver is covered; either a bounded selected corpus reaches the S2 checkpoint with normalized complete-or-transferred form dispositions, or its first unavailable requirement is recorded with the next receiver. No unsupported physical or L3 claim is made. |
| Original Owner Request | Continue the Queue-governed DeskPro work; use reusable production boundaries rather than test-only APIs; owner-managed ROM/media are available for bridge/differential work but must not enter the repository; do not let whole-machine emulator issues block progress. |
| Similar-Issue Sweep | Inspect every Model-40 startup media insertion/caller, helper invocation and capture terminal path; inspect all changed FDD argument validation branches and every emitted observation terminal reason for containment, nonpublication and no-identity leakage. |

## Current Technical Baseline

- **Current developer artifact:** T390 S4 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `8618F97B6987A4A0D12D932A78346F7F72DD4F1E07D2A7F1188D35889AB271ED`.
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
| T390 S4 | Accepted: P1--P3 add the contained Model-40 preboot observer tool, qualify exact real-mode `CLI`, `SAHF` and register-source `MOV Sreg,r/m16` rows, and make first unallocated capture terminal. The full gate, specialized gates and documentation governance pass; the complete boot-sector corpus and every physical profile/board claim remain T390 work. [Evidence](../etc/evidence/t390-s4-model40-preboot-retirement-matrix.md). |
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
