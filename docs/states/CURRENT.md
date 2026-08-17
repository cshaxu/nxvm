# Project Status

## Current Work

- **Active packet:** T386 S28; implement the source-determined shared VADP
  Odd/Even low/high display-page receiver, then prove its fixed Model-40
  binding. No board timing or L3 claim is in scope.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T386 remains the latest open numeric task through its retained S27 progress, and S28 is the next unused subtask. |
| Admission And Approval | Owner's continuing authorization to implement and push the DeskPro 386 functional/L3 program, reaffirmed 2026-08-16 by directing autonomous root-cause work; approved scope is the next proposal-ordered Model-40 CECG functional receiver. |
| Objective | Implement the primary-source-determined CECG `3C2h` bit-5 Odd/Even low/high display-page behavior at the shared VADP owner, including page state, memory address selection, copied-snapshot consumption, reset and the fixed Model-40 declaration. |
| Non-goals | No IBM EGA or VGA breadth claim; no copied VADP core; no generic YAML variant surface; no analog-monitor, cable, raster, service-duration, ISA contention/wait, physical-memory or L3 claim; no option-ROM/BIOS behavior; no vendor ROM, manual scan, guest media, local path, hash or third-party-source commit. |
| Reference Baseline | Accepted T386 S6, S9--S13 and S27 CECG residual audit; S21 matrix; S26 technical baseline; the Model-40 functional-closure proposal; current CECG TODO ledger; and transient primary-guide research under source policy. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | Shared Core VADP state/configuration, memory and snapshot route; Model-40 fixed composition declaration; focused owner and Model-40 tests; CMake, evidence/index/status as required. No new generic variant, copied state or test-only production API. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `DOCUMENT.md`, `ARCHITECTURE.md`, `CODING.md`; `docs/design/ARCHITECTURE.md`, `CODING.md`, `UI.md`; source policy. Core owns reusable VADP state/address/snapshot behavior; VM owns the fixed Model-40 declaration and composition. |
| Verification | Prove low/high page selection only when the configured sequencer/graphics Odd/Even path is active; prove distinct memory and copied-frame visibility, reset to the Model-40 declaration, generic-EGA `3C2h` isolation and retained CECG controls. Run focused tests, documentation governance, specialized gates and current smoke gate. |
| Expected Markers | `M5:T386:S28:CECG-ODD-EVEN-PAGE:OK`; `M5:T386:S28:MODEL40-CECG-ODD-EVEN:OK`; retained S9--S13 CECG markers. |
| Asset Needs | Transient primary manual research only under source policy; project-owned synthetic test data only. No external firmware, media, manual content or local asset path enters Git. |
| Reporting Requirements | Report the actual source-determined address/page semantics, shared owner/root cause, Model-40 declaration, immutable P commit, focused and required-gate conclusion, and explicit physical/firmware/timing transfer without an L3 claim. |
| Stop Conditions | Stop and transfer a behavior if the primary material does not determine it, if it requires physical/electrical/timing representation, or if it needs firmware behavior or generic-profile policy. Repair a reproduced reusable VADP lifecycle or registration defect only at its earliest owner with a personality/caller sweep. |
| Exit Criteria | The sole S27 shared CECG functional receiver has one VADP-owned page-state, memory and copied-snapshot path, the Model-40 declaration/reset binding is proven, generic EGA remains isolated, and all retained physical/firmware/timing items remain explicitly transferred. No L3 claim. |
| Original Owner Request | Continue the approved DeskPro 386 work autonomously and identify the root cause rather than repeatedly requesting decisions. |
| Similar-Issue Sweep | Sweep all VADP planar-memory address/plane-selection paths, sequencer and graphics Odd/Even controls, CECG `3C2h` writers/reset declarations, copied snapshots, generic personality isolation, Model-40 composition/tests, CMake registrations, proposal/matrix/TODO and existing CECG controls. |
## Current Technical Baseline

- **Current developer artifact:** T386 S28 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `80EC4B58630B163ECAFED6FE5B97F3B69D5E22D87928811FE66A8D5274E64EDD`.
  The active S28 implementation has passed its full current-source gate for the
  shared CECG Odd/Even page receiver; acceptance remains pending in
  [S28 evidence](../etc/evidence/t386-s28-cecg-odd-even-page.md).
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
| T386 S27 | Accepted CECG residual audit: source-determined `3C2h` bit-5 Odd/Even display-page behavior is an unimplemented shared VADP mechanism and remains the next T386 receiver; physical monitor, live-video, firmware and timing effects transfer explicitly. No L3 claim. [Evidence](../etc/evidence/t386-s27-cecg-residual-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
