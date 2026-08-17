# Project Status

## Current Work

**Current progress:** T388 S7 is accepted: the existing 80286 `LSL` classifier
and owner smoke retain its Intel-primary fixed register/memory `14/16` rows;
S6's cross-CPU descriptor-granularity receiver was corrected prospectively
without rewriting its history. Descriptor page granularity remains only an
80386 receiver. Prefix/default, x87 and physical-service routes remain open.
No profile is physically enabled and DeskPro board timing remains blocked on
T388 completion.

## M5 T388 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S7 is accepted retained progress for latest open T388. |
| Admission And Approval | Owner authorization to continue according to Queue, 2026-08-16. S7 reconciles 80286 ownership and transfers the genuine 80386 `LSL` descriptor/page context. |
| Objective | Add a private, nonpublishing successful-80386-`LSL` descriptor-granularity capture, then let the sole post-refresh source publisher select Intel-primary byte/page and register/memory rows `20/21` or `25/26`. |
| Non-goals | No descriptor semantic repair, selector/table redesign, generic prefix allocation, fault/delivery timing, CPU physical contract enablement, profile clock/rate, bus/device timing, ROM/media import, public ABI, artifact revision or L3 claim. |
| Reference Baseline | Intel 80386 PRM sections 5/6 and 17.2.2.3; T359 S6; T366 S28; T388 S3--S7; `LSL_R32_RM32`, sole instruction-cost publisher and existing LAR/LSL semantic smoke. |
| Candidate Proposal | [Four-profile CPU physical-timebase closure](../proposals/m5-four-profile-cpu-physical-timebase-closure.md). |
| Files And ABI Surface | Private Core instruction-state/execution capture, sole 80386 source classifier, focused owned smoke, evidence/history/current/index/static gate. No public header, VM/profile or consumer ABI. |
| Applicable Rules | EXECUTION, ARCHITECTURE, CODING, DOCUMENT and source policy. Capture must be reset/reused safely, must be unavailable unless a successful 80386 `LSL` has read its descriptor, and the classifier must retain unallocated prohibition for every other result. |
| Verification | Prove 80386 protected valid `LSL` register/memory against byte-G and page-G descriptors; expect `20/21` and `25/26`; prove invalid selector, non-80386, non-`LSL`, prefix, fault and reset paths cannot select a row; run focused smokes, static gate, documentation governance and current gate. |
| Expected Markers | `M5:T388:S8:80386-LSL-GRANULARITY-CAPTURE:OK`. |
| Asset Needs | Intel primary manual citations and owned test fixtures only; no ROM, guest media, emulator run, trace or machine-local path. |
| Reporting Requirements | Name capture lifecycle and owner, exact selected rows and tests, unavailable-path disposition, source scope, every similar capture/publication hit and remaining T388 receivers. |
| Stop Conditions | Stop and transfer a source conflict, descriptor semantic defect, missing successful-retirement lifecycle hook, public ABI requirement, physical scalar/clock requirement, selector/table/fault redesign or bus/device prerequisite. |
| Exit Criteria | A successful eligible 80386 `LSL` alone can select the exact primary row from captured G and ModR/M state; no stale or unavailable capture can publish it; all other routes remain explicitly unallocated/nonphysical. |
| Original Owner Request | Continue autonomously according to Queue toward full DeskPro functionality and L3 timing; use authoritative Intel sources first and find root causes without routine approval requests. |
| Similar-Issue Sweep | Search all tracked production source, tests, build descriptions and T388 evidence for descriptor reads, `_IsDescSegGranularLarge`, `LSL_R32_RM32`, post-refresh timing capture, `0F 03`, source publication and reset/finalize paths; classify every hit. |
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
| T388 S7 | Accepted: P1 reconciles the existing Intel-primary 80286 `LSL` register/memory `14/16` owner and removes S6's cross-CPU descriptor-granularity receiver prospectively. 80386 descriptor context, prefix/default, x87 and physical-service receivers remain open; no profile is physically enabled. [Evidence](../etc/evidence/t388-s7-80286-lsl-architecture-reconciliation.md). |
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
