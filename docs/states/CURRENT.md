# Project Status

## Current Work

**Active packet:** T390 S15 (Continuation) -- 80386 operand-size RCL-CL qualification.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation -- T390 remains latest open task; S15 follows accepted S14. |
| Admission And Approval | Owner standing approval for Queue-ordered T390 continuation; scope is observed `66 D3 /2` register RCL with CL count. |
| Objective | Qualify exactly the observed operand-size 32-bit register RCL-CL form at Intel's nine-clock register row. |
| Non-goals | No memory, immediate, native-size, other rotate, profile physical enablement, board timing or L3 claim. |
| Reference Baseline | Accepted S13 artifact and Intel 80386 PRM RCL table printed page 372. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | Private Core ledger/classifier, focused smoke, T390 evidence/status only; no public ABI. |
| Applicable Rules | Execution, architecture, coding, documentation, source policy; Intel-primary exact row and unsupported-success rejection. |
| Verification | Focused physical admission at nine ticks plus excluded shapes; bounded redacted replay, current gate, docs, diff, actual review. |
| Expected Markers | Timing-ledger success and normalized capture only. |
| Asset Needs | Owner-managed external assets only; no external data in Git. |
| Reporting Requirements | Complete P1, coordinator review, acceptance/correction. |
| Stop Conditions | Missing exact row/capture/owner, fault, or next unallocated terminal. |
| Exit Criteria | Exact form source-cited, one owner, excluded shapes reject, required proof/evidence/acceptance complete. |
| Original Owner Request | Continue shared 80386 qualification before DeskPro board timing. |
| Similar-Issue Sweep | Inspect D3 group-2 paths, RCL count variants and existing tests. |

## Current Technical Baseline

- **Current developer artifact:** T390 S13 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `9810495E3258E8395050C55452E4DEC20FD361239F2D9F143076DB2CBC7BA227`.
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
| T390 S14 | Accepted corrective: S13 capture proof explicitly distinguishes 7C00 checkpoint success from budget exhaustion; corrected evidence records the new unallocated RCL-CL terminal. Physical-profile selection, board timing and L3 remain open. [Evidence](../etc/evidence/t390-s13-corpus-checkpoint.md). |
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
