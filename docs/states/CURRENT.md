# Project Status

## Current Work

- **Next admission:** continue T386's selected-device functional matrix with
  the next unclosed CECG receiver. No implementation work proceeds until its
  approved packet is installed.

## Current Technical Baseline

- **Current developer artifact:** T386 S26 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `6FAE5FDED1C1264BD8C0609BBC556EA74839E40C2AF2B59DDB5CFE8626B02A0A`.
  S26 preserves the artifact and accepts the fixed Model-40 startup-HDC
  receiver in [S26 evidence](../etc/evidence/t386-s26-model40-fixed-hdd.md).
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
| T386 S26 | Accepted the selected Compaq 40 MB fixed-startup-HDC receiver: Model-40 construction alone accepts the exact 925/5/17 RAW-IMG and rejects later replacement; normal CHS/PIO, IRQ14, reset and error consumer behavior plus shared HDD geometry lifecycle are proven. Physical media and timing remain transferred; no L3 claim. [Evidence](../etc/evidence/t386-s26-model40-fixed-hdd.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
## M5 T386 S27 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T386 remains the latest open numeric task through its single retained S26 progress row, and S27 is the next unused subtask. |
| Admission And Approval | Owner's continuing authorization to implement and push the DeskPro 386 functional/L3 program, reaffirmed 2026-08-16 by directing autonomous root-cause work; approved scope is the next proposal-ordered Model-40 CECG functional receiver. |
| Objective | Close every remaining primary-source-determined Compaq Enhanced Color Graphics/Color Monitor register, state, reset, isolation and selected-consumer gap through the shared VADP personality and fixed Model-40 declaration, or truthfully transfer an underdetermined physical, firmware or timing question. |
| Non-goals | No IBM EGA or VGA expansion; no copied VADP core; no analog-monitor, cable, raster, service-duration, ISA contention/wait or L3 claim; no option-ROM/BIOS behavior without a source-backed contract; no vendor ROM, manual scan, guest media, path, hash or third-party source commit. |
| Reference Baseline | Accepted T386 S6 and S9--S13 CECG contracts, S21 current functional matrix, S26 technical baseline, the Model-40 functional-closure proposal, and current CECG TODO ledger. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | Expected shared Core VADP state/ports/configuration and focused CECG tests, plus Model-40 composition only for fixed historical declaration, CMake/evidence/status as required. No new generic variant or test-only production API. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `DOCUMENT.md`, `ARCHITECTURE.md`, `CODING.md`; `docs/design/ARCHITECTURE.md`, `CODING.md`, `UI.md`; source policy. Core owns reusable VADP register/state behavior; VM owns the Model-40 declaration and composition. |
| Verification | First audit the primary-guide-determined residual observables against current VADP and TODO. Add focused shared-owner and Model-40 consumer/reset/isolation proof only for determined behavior; retain S9--S13 controls. Re-run documentation governance, specialized gates and the current smoke gate. |
| Expected Markers | New `M5:T386:S27:CECG-RESIDUAL:OK` and Model-40 binding marker if implementation is warranted; retained S9--S13 CECG markers. |
| Asset Needs | Transient primary manual research only under source policy; project-owned synthetic test data only. No external firmware, media, manual content or local asset path enters Git. |
| Reporting Requirements | Report each residual observable's source disposition, actual shared owner/root cause, immutable P commit, focused and required gate conclusion, and explicit physical/firmware/timing transfer without an L3 claim. |
| Stop Conditions | Stop and transfer a residual item if the primary material does not determine its semantics, if it requires physical/electrical/timing representation, or if it would need firmware behavior or a generic profile/variant policy. Repair a reproduced reusable VADP lifecycle or registration defect only at its earliest owner with a variant/caller sweep. |
| Exit Criteria | The selected CECG functional ledger has no unclassified primary-determined register/state/reset/consumer gap: each is implemented and proven through shared VADP plus Model-40 binding, or explicitly transferred under the stated boundary. Generic EGA remains isolated and no physical, firmware, timing or L3 behavior is claimed. |
| Original Owner Request | Continue the approved DeskPro 386 work autonomously and identify the root cause rather than repeatedly requesting decisions. |
| Similar-Issue Sweep | Sweep all CECG/VADP personality declarations, `3C2h`/`3C6h`/`3xA`/status/monitor routes, reset and provider-query paths, Model-40 declarations, generic EGA isolation, CECG tests, CMake registrations, proposal/matrix/TODO references and firmware-mode callers. |
