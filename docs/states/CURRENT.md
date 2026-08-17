# Project Status

## Current Work

**Active.** T390 S34 corrects a physical-retirement scope mismatch: the selected Model-40 physical contract publishes every source-classified retirement, although S28 admitted only C0. The repair restores deterministic retirement until a complete later corpus has an enforceable qualification mechanism. No Core/VM interface, board-time, or L3 claim follows.

## M5 T390 S34 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T390 remains the latest open numeric task and S34 follows accepted S33. |
| Admission And Approval | Owner authorized continued implementation, correction of confirmed defects, and CPU/timing audit in the current task conversation on 2026-08-17. |
| Objective | Correct premature Model-40 physical-retirement selection so a C0-only accepted corpus cannot publish elapsed/device time for later source-classified forms before their own complete qualification. |
| Non-goals | No Core/VM public interface change, runtime stage tracker, CPU decoder/executor change, timing-row allocation, board-ratio/device-time claim, asset/media import, or raw capture. |
| Reference Baseline | T390 S28 selected physical retirement after C0; S33 observes a later source-classified post-C0 port-`61h` read; current Core physical publication rejects only source-unallocated retirement. |
| Candidate Proposal | Continue the accepted [T390 physical-qualification proposal](../proposals/m5-80386-physical-retirement-qualification.md) by restoring its stage-bound nonpublication requirement. |
| Files And ABI Surface | Model-40 session construction, its composition regression, current status and evidence only. No Core interface/layout or VM-to-Core mutable-state path. |
| Applicable Rules | Execution, architecture, coding, documentation and source-policy authorities; Core owns generic publication and VM composition selects the contract, while tests prove the composed boundary without exposing a new API. |
| Verification | Build/run the focused Model-40 composition regression and selected retirement-contract regression; prove both Model-40 construction paths select deterministic retirement; run documentation/diff gates and an aggregate-only contained replay if needed. |
| Expected Markers | Existing Model-40 composition marker plus an S34 deterministic-selection assertion; no new timing or board marker. |
| Asset Needs | None for the focused repair. A replay, if run, uses read-only owner-managed assets transiently and retains no asset identity, path, hash, bytes, PC, trace, or media. |
| Reporting Requirements | Record the static reproducer, both composition paths, corrected selection, focused proof, similar-issue sweep, and transfer; state explicitly that classified source rows alone are not a physical stage qualification. |
| Stop Conditions | Stop for a need to add a public stage interface, change Core publication semantics for all profiles, assign physical time, or infer board timing. |
| Exit Criteria | Both Model-40 construction paths remain functional but select deterministic retirement; focused proofs pass; all physical selections are classified by scope; documentation and diff gates pass. |
| Original Owner Request | Continue the global CPU instruction/timing audit toward later DeskPro board timing and L3 closure; fix confirmed issues and preserve Core/VM interfaces. |
| Similar-Issue Sweep | Search all Model-40 construction and physical-contract selections plus their regressions; every production hit is corrected or explicitly outside the Model-40 selection scope. |

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
| T390 S33 | Accepted: test-only aggregate post-C0 I/O capture reaches the immediate-port `61h` read, assigns it to the existing D4 Core mechanism selected by Model-40 composition, and preserves all Core/VM interfaces. No defect, timing, board-time, physical-retirement, or L3 claim. [Evidence](../etc/evidence/t390-s33-post-c0-io-owner-boundary.md). |
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
