# Project Status

## Current Work

- **Current progress:** T387 S5 is accepted: Compaq D3PE confirms that
  port-`61h` bit 4 is the counter-1 refresh observation while the D4 NMI
  latches remain independent. The shared elapsed-time axis still contains
  successful one-tick sentinels, so Model-40 CPU-to-PIT/DCLK calibration and
  physical/L3 conclusions transfer to the queued four-profile physical-timebase
  closure; no timing behavior changed.
## M5 T387 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S6 is the task-level closure and transfer audit for open T387 after accepted S1--S5. |
| Admission And Approval | Owner continuing authorization for DeskPro L3 work and push, 2026-08-16; S5's source-backed shared-timebase finding requires truthful task closure rather than an invented Model-40 physical clock. |
| Objective | Independently reconcile every T387 board/device timing requirement, delivered fact and transfer, then close T387 without claiming Model-40 L3 or physical-time completion. |
| Non-goals | No CPU timing repair, clock scalar, device repair, reference execution, ROM/media import, generic-AT substitution, artifact revision, or L3 claim. |
| Reference Baseline | T384 S1/S2, T385 S1, T386 S15/S22--S29, T387 S1--S5, four-profile physical-timebase proposal, DeskPro board and model-L3 proposals, source policy and execution rules. |
| Candidate Proposal | [DeskPro 386 board bus and device timing closure](../proposals/m5-deskpro-386-bus-device-closure.md). |
| Files And ABI Surface | T387 closure evidence/current/index and any required Queue/proposal transfer reconciliation; no source, ABI, profile, artifact, asset or media change. |
| Applicable Rules | EXECUTION/DOCUMENT/ARCHITECTURE/CODING, architecture/coding/UI, source policy and differential-debug policy. |
| Verification | Map every T387 proposal requirement to S1--S5 evidence/current source or one named receiver; inspect the actual pushed S5 changes and current Queue order; run documentation governance and git diff check, retaining S5 P1's 281-target current-gate proof. |
| Expected Markers | `M5:T387:S6:TASK-CLOSURE-TRANSFER-AUDIT:OK`. |
| Asset Needs | No asset use; retained source-labelled evidence only. |
| Reporting Requirements | State exactly what T387 delivered, which primary facts are retained, every unresolved board/CPU/device receiver, and why no Model-40 L3 result is claimed. |
| Stop Conditions | Stop and record a Queue/TODO receiver if any T387 requirement lacks a named owner, if a new board behavior needs CPU-timebase repair, or if the closure would imply physical timing or L3. |
| Exit Criteria | One independent closure matrix covers all T387 scope, current Queue dependencies are sufficient and ordered, documentation governance passes, and T387 closes only with explicit transfers. |
| Original Owner Request | Continue autonomously toward full DeskPro function and L3 timing; find and resolve root causes without asking routine design questions. |
| Similar-Issue Sweep | Sweep all T387 evidence, T386 timing transfers, T366/T368 physical-time records, Model-339 virtual-time evidence, Queue and TODO for unnamed CPU-to-board-clock, device-service or L3 transfers. |
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
