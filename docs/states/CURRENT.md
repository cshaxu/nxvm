# Project Status

## Current Work

## M5 T388 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; T388 is the first queued shared physical-timebase candidate after closed T387. |
| Admission And Approval | Owner continuing authorization for autonomous L3 work and Git push, 2026-08-16; T387 S5/S6 explicitly transfer this prerequisite. |
| Objective | Build the source, publisher and clock-domain inventory required to define the shared physical-timebase closure without allocating a scalar. |
| Non-goals | No cycle value, CPU semantic repair, board/device timing implementation, profile clock ratio, host-time coupling, ROM/media import, artifact revision or L3 claim. |
| Reference Baseline | T359/T360/T363 timing closures, T366 S9/S10/S32, T368 S7, T375 S17, T387 S5/S6, physical-timebase proposal, source policy and execution rules. |
| Candidate Proposal | [Four-profile CPU physical-timebase closure](../proposals/m5-four-profile-cpu-physical-timebase-closure.md). |
| Files And ABI Surface | Evidence/current/index/static inventory plus the owner-requested Queue/proposal dependency correction; no public or runtime ABI, profile, asset or media change. |
| Applicable Rules | EXECUTION/DOCUMENT/ARCHITECTURE/CODING, architecture/coding, source policy and differential-debug policy. |
| Verification | Inventory every successful elapsed-time publisher/classifier and every physical-clock consumer across four profiles; prove the current sentinels/reachable paths and classify each earliest receiver; run focused static checks, documentation governance and applicable current-gate verification. |
| Expected Markers | `M5:T388:S1:PHYSICAL-TIMEBASE-INVENTORY:OK`. |
| Asset Needs | No asset use; retained Intel/Compaq-labelled evidence and current source only. |
| Reporting Requirements | Distinguish source-backed cycle rows, explicit sentinels, non-retiring paths and host pacing; record no physical-clock conclusion in S1. |
| Stop Conditions | Stop and transfer any source range or unavailable form context rather than infer a cycle count; stop if a consumer requires a board/device contract rather than the shared timebase. |
| Exit Criteria | A complete four-profile publisher-to-consumer matrix identifies all current physical-timebase blockers and the exact bounded next implementation receiver. |
| Original Owner Request | Continue autonomously toward full DeskPro function and L3 timing; find and resolve root causes without routine approval requests. |
| Similar-Issue Sweep | Search all tracked Core/VM production, tests, CMake gates and timing evidence for elapsed ticks, unallocated timing sentinels, clock plans, virtual-time sources and profile frequency claims. |
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
