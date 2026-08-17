# Project Status

## Current Work

- **Current progress:** T387 S4 accepts the local-reference negative
  disposition: 86Box and PCjs cannot supply the exact Model-40 PIT counter-1 /
  port-`61h` receiver, and local MAME material is unavailable. S5 reconciles
  the primary port-`61h` facts and the inherited physical-time non-admission;
  it must transfer the shared CPU timebase blocker rather than fabricate a
  16 MHz calibration. No timing behavior changed.
## M5 T387 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S5 reconciles the apparent S3/S4 port-61h conflict within open T387. |
| Admission And Approval | Owner continuing authorization for DeskPro L3 work and push, 2026-08-16; discovered in-scope source reconciliation requires a truthful disposition before further timing work. |
| Objective | Reconcile the Model-40 port-61h/PIT source boundary against D3PE and define the generic virtual-time receiver that can consume its primary clock facts. |
| Non-goals | No reference execution, ROM/media import, generic-AT substitution, rewrite of historical evidence, physical waveform claim, or L3 claim. |
| Reference Baseline | T384 S1 primary D4 audit, T386 S3/S22/S23, T387 S1-S4, source policy, differential-debug policy and DeskPro board proposal. |
| Candidate Proposal | [DeskPro 386 board bus and device timing closure](../proposals/m5-deskpro-386-bus-device-closure.md). |
| Files And ABI Surface | Evidence/current/index plus Queue/proposal transfer records, and a bounded generic Core/VM virtual-time implementation only if the existing architecture admits it; no profile-local clock mechanism, asset or ABI change. |
| Applicable Rules | EXECUTION/DOCUMENT/ARCHITECTURE/CODING, architecture/coding/UI, source policy and differential-debug policy. |
| Verification | Reconcile every port-61 and PIT assertion against retained primary-labelled evidence; inventory the existing virtual-time/clock-domain owner and admit only a source-backed generic receiver; run focused/full verification and documentation governance. |
| Expected Markers | `M5:T387:S5:PORT61-PIT-RECONCILIATION:OK`; any implementation marker must name the generic virtual-time owner. |
| Asset Needs | No asset use; retained source-labelled evidence only. |
| Reporting Requirements | Preserve S3/S4 historically, state why port-61 bit 4 and D4 NMI bits coexist, identify the exact virtual-time receiver and all transfers, and record every behavior change. |
| Stop Conditions | Stop and transfer if the existing virtual-time owner cannot consume a board clock without coupling it to host time or successful-retirement count. |
| Exit Criteria | The timing ledger records the primary D4 port-61/PIT facts and either delivers a generic source-backed virtual-time receiver or transfers a precise architectural blocker. |
| Original Owner Request | Continue autonomously toward full DeskPro function and L3 timing; find the root cause rather than requesting a choice. |
| Similar-Issue Sweep | Sweep T387 evidence, DeskPro proposal, T384 D4 audit, retained Model-40 functional evidence and all virtual-time/clock-domain consumers for an inherited generic-PC/AT assumption or retirement-time coupling. |
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
| T387 S4 | Accepted local-reference qualification: checked 86Box/PCjs entries do not prove the exact Model-40 PIT counter-1/port-`61h` route and local MAME material is unavailable. No substitution, scalar or L3 claim. [Evidence](../etc/evidence/t387-s4-model40-reference-qualification.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
