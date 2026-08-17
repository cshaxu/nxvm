# Project Status

## Current Work

- **Current progress:** T387 S3 accepts the bounded Model-40 PIT counter-1 /
  port-`61h` bridge-observation contract. The next receiver must qualify an
  exact Model-40 reference or record its negative disposition; no timing
  behavior changed.
## M5 T387 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T387 S3 is accepted progress and S4 is next. |
| Admission And Approval | Owner continuing authorization for DeskPro L3 work and push, 2026-08-16; S3 assigns exact-reference qualification or negative disposition. |
| Objective | Qualify locally available secondary reference candidates for the exact 1986 Compaq DeskPro 386 Model 40 bridge contract, or record their bounded negative disposition. |
| Non-goals | No product code, reference execution, trace capture, ROM/media import, generic-AT substitution, scalar, timing repair or L3 claim. |
| Reference Baseline | T387 S1-S3, T384 primary Model-40 audit, source policy, differential-debug policy, and locally installed owner-managed reference material. |
| Candidate Proposal | [DeskPro 386 board bus and device timing closure](../proposals/m5-deskpro-386-bus-device-closure.md). |
| Files And ABI Surface | Evidence/current/index only; no source, CMake, ABI, asset or test change. |
| Applicable Rules | EXECUTION/DOCUMENT/ARCHITECTURE/CODING, architecture/coding/UI, source policy and differential-debug policy. |
| Verification | Search local reference source/configuration assets and record exact model identifiers, board-route support evidence, exclusions and a reproducible result. |
| Expected Markers | `M5:T387:S4:MODEL40-REFERENCE-QUALIFICATION:OK`; no runtime timing marker. |
| Asset Needs | Read-only inspection of owner-managed local reference sources/configurations; no bytes copied, modified or committed. |
| Reporting Requirements | Identify every checked candidate and revision/path class without publishing machine-local paths; distinguish an unsupported exact model from uninspected or non-comparable candidates. |
| Stop Conditions | Stop with a negative disposition if no checked candidate proves both exact Model-40 board identity and the counter-1/port-61h route. |
| Exit Criteria | A future bridge run has one qualified reference candidate with both required identities, or a documented negative result transfers the receiver without an AT substitution. |
| Original Owner Request | Continue autonomously toward full DeskPro function and L3 timing. |
| Similar-Issue Sweep | Sweep 86Box, MAME, PCjs and local source/configuration manifests for aliases, generic-AT fallbacks, DeskPro variants and port-61h route evidence. |
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
| T387 S3 | Accepted a bounded Model-40 PIT counter-1/port-`61h` bridge-observation contract: exact-reference qualification, compact checkpoints, masks, budgets and cleanup are mandatory; it produces no scalar or L3 claim. [Evidence](../etc/evidence/t387-s3-model40-bridge-observation-contract.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
