# Project Status

## Current Work

- **Active packet:** T387 S2; source/probe disposition for the Model-40 CPU-to-PIT and DCLK clock conversions.
  it establishes the source/owner gaps and transfers exact clock conversion to
  the next S. No timing behavior changed.

## M5 T387 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T387 S1 is accepted compact progress and S2 is the next unused subtask. |
| Admission And Approval | Owner continuing authorization for autonomous DeskPro 386 functional/L3 implementation and push, reaffirmed 2026-08-16; the S1 ledger explicitly assigns this source/probe disposition. |
| Objective | Determine whether primary material plus a bounded project-owned observation can establish the Model-40 CPU-to-PIT conversion and 4 MHz DCLK conversion needed for a later Core clock-plan mechanism, or record a precise non-admission. |
| Non-goals | No timing implementation, arbitrary scalar, profile/device functional repair, external ROM/media import, secondary-emulator authority, waveform/L3 or Windows claim. |
| Reference Baseline | T384 primary-source audit, T386 S25 CPU-to-PIT transfer, T386 S17 DMA facts, T387 S1 inventory, board-timing proposal, source policy and current Core clock/timeline owner. |
| Candidate Proposal | [DeskPro 386 board bus and device timing closure](../proposals/m5-deskpro-386-bus-device-closure.md). |
| Files And ABI Surface | Evidence/current/history/TODO reconciliation only; no production, CMake or test change. |
| Applicable Rules | EXECUTION, DOCUMENT, ARCHITECTURE and CODING rules; project architecture/coding/UI; source policy. Preserve one Core clock/timeline owner and distinguish primary facts, qualified observation and unmeasured hardware. |
| Verification | Inspect primary Compaq/Intel material transiently, current ratio consumers and existing probes; document exact source, domain, conversion/unknown, observation limits and next receiver; run documentation governance. |
| Expected Markers | `M5:T387:S2:MODEL40-CLOCK-SOURCE-DISPOSITION:OK`; no runtime timing or L3 marker. |
| Asset Needs | Transient public documentation only when needed; do not commit vendor bytes, ROM/media, local paths, unreviewed source or raw captures. |
| Reporting Requirements | Record primary authority, all equations or their absence, current owner/consumer sweep, secondary-reference limit, no-invention conclusion and next implementation/non-admission receiver. |
| Stop Conditions | Stop with exact non-admission if no primary-supported conversion plus bounded observation exists; do not derive a scalar from "below 8 MHz", guest boot behavior or secondary emulator alone. |
| Exit Criteria | Every CPU-to-PIT/DCLK conversion need has one source/probe disposition and a named later mechanism; no source fact is promoted beyond its proven clock domain. |
| Original Owner Request | Continue autonomously toward DeskPro 386 complete function and L3 timing under governance. |
| Similar-Issue Sweep | Sweep T384/T386/T387 evidence, Core clock plans/domains/timeline consumers, Model-40 composition, selected tests/probes, primary-source links and all Queue/TODO timing transfers. |
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
| T387 S1 | Accepted source-labelled Model-40 board/device timing inventory; DCLK and CPU-to-PIT conversion remain the next evidence receiver, while no timing behavior or L3 claim was made. [Evidence](../etc/evidence/t387-s1-model40-board-timing-inventory.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
