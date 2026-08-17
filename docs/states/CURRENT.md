# Project Status

## Current Work

- **Active packet:** T386 S29; independently audit the completed selected-device
  functional matrix, fixed-versus-external declarations and precise board/
  firmware/physical timing handoff. No source repair or timing implementation
  is in scope unless the audit reproduces an in-scope false completion. The audit
  reproduced one false-completion finding: S26's HDC regression was built but
  omitted from the canonical current gate; this packet revision admits only its
  CMake registration and replay.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T386 remains the latest open numeric task through retained S28 progress, and S29 is the next unused subtask. |
| Admission And Approval | Owner's continuing authorization to implement and push the DeskPro 386 functional/L3 program, including autonomous root-cause work, reaffirmed 2026-08-16. The approved scope is the proposal-ordered selected-device functional-closure audit. |
| Objective | Independently reconcile every selected Model-40 functional row against accepted S21--S28 evidence and current source/tests; accept transfer to DeskPro board/device timing only if every selected device has an implemented functional owner and proof or a truthful explicit firmware/physical boundary. |
| Non-goals | No board/device timing allocation, L3 acceptance, generic clone/variant work, IBM EGA/VGA breadth, new BIOS/ROM/media behavior, external asset import, or implementation unrelated to a reproduced false-completion finding. |
| Reference Baseline | T384 capability audit, T385 CPU closure, S21 matrix, accepted S22--S28 evidence, current T386 proposal/TODO, Queue/Roadmap, and S28 artifact `80EC4B58630B163ECAFED6FE5B97F3B69D5E22D87928811FE66A8D5274E64EDD`. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | `CMakeLists.txt` and its T345 ownership verifier may add the existing S26 HDC smoke to the canonical current gate and update their resulting strict-test inventory; expected evidence, proposal/current/history/TODO reconciliation follows. No production ABI, device implementation or timing change is admitted. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `DOCUMENT.md`, `ARCHITECTURE.md`, `CODING.md`; `docs/design/ARCHITECTURE.md`, `CODING.md`, `UI.md`; source policy. Audit one owner and proof per selected state/register/reset/IRQ-DMA/consumer row; preserve Core/VM separation and no unsupported L3 inference. |
| Verification | Inspect actual source/registrations and replay the retained targeted Model-40/D4/FDC/HDC/CECG controls plus current documentation governance and full current-source gate. Verify S26 is registered and executed by the canonical gate. Build a requirement-to-evidence matrix that distinguishes complete function, board timing, firmware lifecycle and physical-media debt. |
| Expected Markers | `M5:T386:S29:FUNCTIONAL-CLOSURE-AUDIT:OK`; retained S22--S28 markers and current gate. |
| Asset Needs | No external asset is needed. Existing evidence only; if source research becomes necessary, use transient material under source policy and do not commit vendor/manual/ROM/media content or local paths. |
| Reporting Requirements | Record every selected row, current owner/proof, acceptance or exact transfer, all audit findings, actual commits/gate result, and an explicit no-L3 conclusion. |
| Stop Conditions | The reproduced S26 gate-registration omission is this packet's sole admitted corrective instance; stop and admit a narrow corrective S for any further source/test finding that disproves a completed functional row; transfer an underdetermined physical, firmware or timing fact without invention; do not reclassify a known board/physical boundary as functional completion. |
| Exit Criteria | One matrix disposes every selected Model-40 functional row as complete with source/current proof or as a named, truthful later receiver; no selected device remains only syntactically composed; the next Queue candidate is the DeskPro board bus/device timing closure, while T386 itself makes no L3 claim. |
| Original Owner Request | Continue the approved DeskPro 386 work autonomously and identify the root cause rather than repeatedly requesting decisions. |
| Similar-Issue Sweep | Sweep T384/T385/S21--S28 evidence, Model-40 composition/profile and current-gate tests, including every selected-device smoke target registration; selected CPU/RAM/D4/PIC/DMA/PIT/RTC/8042/FDC/HDC/CECG/ROM routes; fixed/external declarations; Queue/TODO/proposal/history; current supported-device surface and all claimed transfer destinations. |
## Current Technical Baseline

- **Current developer artifact:** T386 S28 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `80EC4B58630B163ECAFED6FE5B97F3B69D5E22D87928811FE66A8D5274E64EDD`.
  T386 S28 accepts the shared CECG Odd/Even page receiver in
  [S28 evidence](../etc/evidence/t386-s28-cecg-odd-even-page.md); the selected
  functional-closure audit remains next.
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
| T386 S28 | Accepted the sole source-determined shared CECG functional receiver: `3C2h` bit-5 now selects low/high Odd/Even page state through VADP memory and copied snapshots; reset restores the explicit Model-40 low-page declaration and generic EGA remains isolated. Physical, firmware and timing effects transfer unchanged; no L3 claim. [Evidence](../etc/evidence/t386-s28-cecg-odd-even-page.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
