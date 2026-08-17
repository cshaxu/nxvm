# Project Status

## Current Work

- **Next admission:** continue T386's selected-device functional matrix with
  the next unclosed receiver. No implementation work proceeds until its
  approved packet is installed.

## Current Technical Baseline

- **Current developer artifact:** T386 S25 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `197BC89BD2476F10D34559DC4EB71A35EEF7F6C58DFA8ECFEE9F8F592817836D`.
  S25 preserves the artifact and accepts the startup-compatibility receiver in
  [S25 evidence](../etc/evidence/t386-s25-rom-alias-mechanism.md).
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
| T386 S25 | Accepted external-ROM startup-compatibility receiver: generic immutable-ROM aliases, bounded original-80386 control-register compatibility, D4 mapping/reset and shared PC/AT refresh wiring are proven without a profile-local CPU path. The optional owner-supplied replay does not reach its self-contained consumer; source-backed CPU-to-PIT calibration transfers to DeskPro board bus/device timing, with no timing or L3 claim. [Evidence](../etc/evidence/t386-s25-rom-alias-mechanism.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
## M5 T386 S26 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T386 remains the latest open numeric task through its single retained S25 progress row, and S26 is the next unused subtask. |
| Admission And Approval | Owner's continuing authorization to implement and push the DeskPro 386 functional/L3 program, reaffirmed 2026-08-16 by directing autonomous root-cause work; approved scope is the next proposal-ordered Model-40 functional receiver only. |
| Objective | Publish the selected Compaq Multipurpose Fixed Disk Controller's fitted 40 MB RAW-IMG route through the fixed Model-40 YAML/session composition, with startup-only media lifetime and a project-owned end-to-end normal CHS/PIO, reset/error and IRQ14 consumer proof. |
| Non-goals | No ATA or IBM MFM substitution; no new generic variant builder; no hot-swap or post-publication fixed-media replacement; no vendor ROM/media commit; no Long/format/ECC/recovery/physical-sector behavior; no controller-service or board timing/L3 claim; no CECG work. |
| Reference Baseline | Accepted T386 S5 Compaq HDC contract, S20 fixed BYOB backbone, S21 functional matrix, S24 FDC receiver, S25 startup receiver, current `vm-0-5-0389` artifact and the Model-40 functional-closure proposal. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | Expected VM composition/profile/catalog/session lifecycle and Model-40 focused tests, plus CMake/evidence/status only as required. Preserve Core's generic HDC API unless a reproduced reusable mechanism defect demands its earliest-owner repair; no public test-only API. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `DOCUMENT.md`, `ARCHITECTURE.md`, `CODING.md`; `docs/design/ARCHITECTURE.md`, `CODING.md`, `UI.md`; source policy. Core retains reusable controller semantics; VM owns Model-40 backbone, media policy and composition. |
| Verification | Add focused Model-40 product/session proof using only project-owned synthetic RAW-IMG input: successful frozen startup path, normal CHS/PIO consumer result, IRQ14 acknowledgement/reset/error behavior, and rejection of fixed-media replacement after publication. Add direct shared-HDD geometry validation, reset-persistence and replacement-reset proof. Re-run retained Compaq HDC S5 and Model-40 FDC/S25 controls, documentation governance, specialized gates and current smoke gate. |
| Expected Markers | New `M5:T386:S26:MODEL40-HDC-STARTUP:OK` and `M5:T386:S26:MODEL40-HDC-FIXED-MEDIA:OK`; retained `M5:T270:S2:MEDIA-PROVIDER:OK`, S5, S24 and S25 markers. |
| Asset Needs | Project-owned synthetic test media only. Optional owner-supplied external ROM/media may be used locally only under the source policy and must not be committed, catalogued, path-recorded or required for acceptance. |
| Reporting Requirements | Report the actual owner/root cause, immutable P commit, focused and required gate conclusion, and explicit transfer of physical media/timing boundary. Record evidence without protected bytes, paths or hashes. |
| Stop Conditions | Stop and transfer if the route requires a vendor firmware/media fact, generic public variant policy, a new physical-media representation, or a timing workaround. Repair a reproduced shared Core lifecycle defect only at its earliest owner with a variant/caller sweep. |
| Exit Criteria | The fixed Model-40 profile accepts one valid startup HDC image only during construction, reaches the selected Compaq normal HDC consumer path with reset/error/IRQ14 proof, rejects later replacement, preserves default/Model-339 isolation, and records remaining physical/timing debt without an L3 claim. |
| Original Owner Request | Continue the approved DeskPro 386 work autonomously and identify the root cause rather than repeatedly requesting decisions. |
| Similar-Issue Sweep | Sweep all `hdd`, `hard_disk`, `vm_session_insert_hdd`, Model-40 private composition, catalog-to-provider argument transport, `hdc_present`, HDC protocol, boot preference and fixed-media lifecycle callers; distinguish default ATA and Model-339 no-disk behavior. |
