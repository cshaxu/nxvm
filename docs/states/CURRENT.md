# Project Status

## Current Work

**Active.** T390 S33 is collecting the first post-C0 I/O semantic event under the corrected-ROM baseline. The work is test-only and aggregate-only: it identifies direction and, when directly encoded, an I/O port after C0 without retaining bytes, PCs, or a raw trace. No timing, board-time, physical-retirement, or L3 claim follows.

## M5 T390 S33 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T390 remains the latest open numeric task and S33 follows accepted S32. |
| Admission And Approval | Owner authorized continued implementation, correction of confirmed defects, and CPU/timing audit in the current task conversation on 2026-08-17. |
| Objective | Add a bounded, test-only aggregate observer that identifies the first retired I/O operation after C0 and its port when directly encoded, so the next semantic checkpoint is assigned to an actual Core or Model-40 device owner rather than inferred from instruction budgets. |
| Non-goals | No production Core/VM API or data-layout change, port-provider replacement, device semantic change, timing allocation, physical-clock or board-ratio claim, ROM/media import, or raw bytes/PC/trace retention. |
| Reference Baseline | Accepted T390 S27/S28 C0 batch and physical-contract selection; accepted S31 corrected A20 startup path; accepted S32 C1 candidate absent within its containment run. |
| Candidate Proposal | Continue the accepted [T390 physical-qualification proposal](../proposals/m5-80386-physical-retirement-qualification.md) through a finite, owner-qualified post-C0 semantic checkpoint. |
| Files And ABI Surface | Existing test-only BYOB capture helper and synthetic smoke, current status and aggregate-only evidence. No production source, Core/VM interface, timing ledger, profile contract, CMake registration, or external source change. |
| Applicable Rules | Execution, coding, architecture, documentation and source-policy authorities; tests may inspect same-module state without creating a public contract, and the Core/VM composition boundary remains unchanged. |
| Verification | Build/run the synthetic post-C0 I/O state regression; execute one contained owner-managed replay with wall and retirement limits; verify only aggregate result is retained and no raw output remains; run focused capture and documentation/diff gates. |
| Expected Markers | Existing C0 and S32 markers plus an S33 synthetic post-C0-I/O marker and an aggregate-only contained summary. |
| Asset Needs | Read-only owner-managed formal assets, used transiently only; no asset identifier, locator, hash, provenance, byte, PC, trace or guest media enters Git. |
| Reporting Requirements | Record the observation definition, first post-C0 I/O event or bounded absence, owner disposition, similar-issue sweep, and transfer without timing inference. |
| Stop Conditions | Stop for an observation that requires a public interface or raw capture, source-unallocated success, a form-capacity condition, or a proposed timing/board conclusion. |
| Exit Criteria | Synthetic observer transitions prove deterministically; contained replay reports a first aggregate post-C0 I/O event (with port when directly encoded) or bounded absence; the event is assigned to an existing owner or transferred; documentation and focused verification pass. |
| Original Owner Request | Continue the global CPU instruction/timing audit toward later DeskPro board timing and L3 closure; fix confirmed issues and preserve Core/VM interfaces. |
| Similar-Issue Sweep | Review every test-only post-checkpoint instruction-inspection path and all existing I/O observation facilities; classify their boundaries so no production hook, duplicate port provider, or raw-output side path is introduced. |

## Current Technical Baseline

- **Current developer artifact:** T390 S22 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `7CD172472BADEEE78160548EDE93104C94874CE03F25DDA2392A63E1217FE2C5`.
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
| T390 S32 | Accepted: aggregate-only C1 phase tracker proves the candidate later real-to-protected transition synthetically, but it is absent within the bounded corrected-ROM replay. The candidate transfers; no timing, board-time, physical-retirement, or L3 claim. [Evidence](../etc/evidence/t390-s32-c1-transition-boundary.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
