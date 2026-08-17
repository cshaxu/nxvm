# Project Status

## Current Work

**Current progress:** T388 S6 is accepted, but its retained `LSL`-granularity
statement requires a narrow corrective reconciliation: 80286 `LSL` already has
an exact register/memory source owner, and descriptor page granularity belongs
to the 80386 architecture. No profile is physically enabled and DeskPro board
timing remains blocked on T388 completion.

## M5 T388 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S6 is accepted retained progress for latest open T388. |
| Admission And Approval | Owner authorization to continue according to Queue, 2026-08-16. Read-only source and classifier review found the S6 evidence's 80286 `LSL` descriptor-granularity receiver conflicts with its actual exact classifier row and the Intel 80286/80386 architectural boundary. |
| Objective | Correctly reconcile the successful 80286 `0F 03 LSL` register/memory Appendix-B rows, retain S6 history without rewriting it, and make the residual ledger and recurrence evidence reject a future cross-CPU descriptor-granularity attribution. |
| Non-goals | No timing scalar change, descriptor semantic change, physical-contract enablement, profile clock/rate, 80386 timing allocation, generic prefix charge, x87 timing, bus/device work, ROM/media import, artifact revision or L3 claim. |
| Reference Baseline | Intel 80286/80287 PRM Appendix B and descriptor architecture; Intel 80386 PRM descriptor-granularity definition; T359 S6; T388 S3--S6; current 80286 classifier and owner smoke. |
| Candidate Proposal | [Four-profile CPU physical-timebase closure](../proposals/m5-four-profile-cpu-physical-timebase-closure.md). |
| Files And ABI Surface | 80286 timing evidence, residual ledger/history/current/index and a narrow static recurrence gate; focused owner smoke only if its current exact register/memory proof needs an assertion clarification. No production ABI or VM/profile surface. |
| Applicable Rules | EXECUTION, ARCHITECTURE, CODING, DOCUMENT and source policy. Intel primary manuals decide CPU-specific architectural/timing facts; evidence must distinguish an exact classified row from an unallocated receiver. |
| Verification | Re-read the actual `0F 03` classifier and owner smoke; source-anchor the 80286 register/memory `14/16` rows; search all active T388 evidence/history/static gates for the false 80286-granularity attribution; run the focused owner smoke, static gate, documentation governance and current gate. |
| Expected Markers | `M5:T388:S7:80286-LSL-ARCHITECTURE-RECONCILIATION:OK`. |
| Asset Needs | Primary manual citations and owned test fixtures only; no ROM, guest media, emulator run, trace or machine-local path. |
| Reporting Requirements | Identify the corrected claim, exact 80286 source/classifier/smoke owner, historical S6 retention boundary, all similar evidence hits and the remaining genuine T388 receivers. |
| Stop Conditions | Stop and transfer any source conflict, attempted 80286 descriptor semantic repair, 80386 timing context, physical scalar/clock requirement, bus/device prerequisite or CPU semantic defect. |
| Exit Criteria | The active evidence truthfully records `LSL` as an exact successful 80286 register/memory row; no active T388 receiver or recurrence gate treats 80386 page granularity as an 80286 timing context; genuine prefix/default, x87 and physical-service receivers remain explicit. |
| Original Owner Request | Continue autonomously according to Queue toward full DeskPro functionality and L3 timing; use authoritative Intel sources first and find root causes without routine approval requests. |
| Similar-Issue Sweep | Search all tracked active T388 source, tests, CMake gates, evidence, history and Current records for `LSL`, `granularity`, `Appendix-B`, `0F 03`, `14/16` and 80286/80386 attribution; record every hit and disposition. |
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
| T388 S6 | Accepted: P1/P2 reconcile completed 80286 Appendix-B source contexts from residual sentinel routes and correct the exact-row/sentinel evidence boundary. `LSL`, prefix/default and x87 receivers remain open; no profile is physically enabled. [Evidence](../etc/evidence/t388-s6-80286-appendix-b-context.md). |
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
