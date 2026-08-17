# Project Status

## Current Work

- **Active packet:** T387 S1; build the source-labelled Model-40 board/device
  timing inventory that bounds all later timing implementation. No timing
  implementation, profile expansion, firmware import or L3 decision is in
  scope.

## M5 T387 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; T386 is closed, the DeskPro board bus/device timing candidate is first in Queue, and T387 is the next numeric implementation task. S1 is its first subtask. |
| Admission And Approval | Owner's continuing authorization to implement and push the DeskPro 386 functional/L3 program, including autonomous root-cause work, reaffirmed 2026-08-16. The approved scope is the next Queue candidate, beginning with its required source-to-model inventory. |
| Objective | Establish one source-labelled inventory of Model-40 board memory/ROM and I/O availability, controller service/IRQ/DRQ/reset timing, arbitration, CPU-to-PIT calibration and every already-complete functional receiver; identify the earliest timing owner and exact proof needed before any implementation. |
| Non-goals | No timing code, waits/delays, L3 acceptance, device-functional rework, generic clone/variant work, external ROM/media import, host-time coupling, waveform claim or Windows claim. |
| Reference Baseline | Closed T384--T386 evidence, especially T386 S29 functional-closure audit and its retained TODO transfers; DeskPro closure context, board-timing proposal, Queue/Roadmap, current Core/VM ownership rules, and current artifact `vm-0-5-0389` / SHA-256 `80EC4B58630B163ECAFED6FE5B97F3B69D5E22D87928811FE66A8D5274E64EDD`. |
| Candidate Proposal | [DeskPro 386 board bus and device timing closure](../proposals/m5-deskpro-386-bus-device-closure.md). |
| Files And ABI Surface | Expected evidence, proposal/current/history/TODO reconciliation only. No production ABI, CMake, source or test change is admitted by S1. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `DOCUMENT.md`, `ARCHITECTURE.md`, `CODING.md`; `docs/design/ARCHITECTURE.md`, `CODING.md`, `UI.md`; source policy. Preserve Core ownership of reusable transactions/events and VM ownership of Model-40 composition; distinguish function from board timing and do not infer unmeasured hardware facts. |
| Verification | Read the applicable primary-source and retained evidence authorities; inspect current timing publishers/consumers and Model-40 routes; produce a component-to-source-to-owner-to-proof matrix with explicit unknowns, cross-reference limits and TODO/Queue receivers. Run documentation governance. |
| Expected Markers | `M5:T387:S1:MODEL40-BOARD-TIMING-INVENTORY:OK`; no runtime timing marker or L3 marker. |
| Asset Needs | No external asset is required for this inventory. If further primary research becomes necessary, use transient material under source policy; do not commit ROM, firmware, guest media, local paths or unreviewed third-party content. |
| Reporting Requirements | Record every selected board/device timing row, source authority or exact uncertainty, current publisher/consumer, future mechanism receiver, owner boundary, verification need and all transfers. State explicitly that no timing behavior changed. |
| Stop Conditions | Stop and transfer any timing fact lacking an authoritative source or bounded probe; admit a later corrective S for a reproduced functional defect; do not convert range-only or secondary-emulator observations into exact timing. |
| Exit Criteria | One complete inventory covers memory/ROM, CPU/PIT calibration, PIC/DMA/FDC/HDC/CECG/RTC/8042/NMI/reset availability and command lifecycle; every row has a source/probe disposition and a future timing owner; the next S can implement one bounded mechanism without reopening T386 functionality. |
| Original Owner Request | Continue the approved DeskPro 386 work autonomously, identify root causes rather than repeatedly requesting decisions, and progress toward full L3 closure. |
| Similar-Issue Sweep | Sweep T386 functional evidence/TODO, shared Core timing publishers and consumers, Model-40 VM composition/profile bindings, current-gate timing tests, all selected controller lifecycle routes, Queue/Roadmap and existing primary/secondary source boundaries. |
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
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
