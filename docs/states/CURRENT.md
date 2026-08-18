# Project Status

## Current Work

**Active: M5 Td S105.**

## M5 Td S105 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance |
| Admission And Approval | Owner approved continuing DeskPro L3/hardware-gap implementation; primary sources first, named reference implementations second, and explicitly labelled generic IBM AT skeletons when neither is available. Owner authorized proposal/Queue changes, repairs, commits and pushes to `master` without repeated confirmation; reaffirmed 2026-08-18. |
| Objective | Repair the post-T405 Queue gap by admitting a tiered DeskPro timing-fallback candidate that converts retained physical receivers into bounded, executable work without changing runtime behavior. |
| Non-goals | No runtime code, artifact identity, L3 decision, ROM/media import, generic behavior masquerading as original Compaq evidence, or reordering of the later generic-PC/XT program. |
| Reference Baseline | T398 physical Model-L3 not-ready decision; T399 reference-derived bridge; T403 keyboard functional closure; T405 host portability closure; current `vm-0-5-0405` baseline. |
| Candidate Proposal | [DeskPro tiered timing fallback foundation](../proposals/m5-deskpro-tiered-timing-fallback-foundation.md). |
| Files And ABI Surface | `docs/proposals/m5-deskpro-tiered-timing-fallback-foundation.md`, `docs/states/QUEUE.md`, `docs/states/CURRENT.md`, and supporting governance evidence only. No ABI or source change. |
| Applicable Rules | Documentation and execution governance; source policy forbids import and requires references remain read-only. Future implementation must also read architecture and coding rules. |
| Verification | Documentation governance; actual Queue/proposal/current-state review; verify the candidate precedes the ineligible post-L3 generic-PC item and names tier, owner, evidence and stop boundaries. |
| Expected Markers | `M5:TD:S105:TIERED-DESKPRO-QUEUE:OK` |
| Asset Needs | No asset, firmware, guest media, local ROM path, binary, or third-party source import. |
| Reporting Requirements | Record why closed T398/T399 evidence cannot be treated as original proof, how PCjs/86Box/generic-AT are permitted as labelled implementation inputs, and the next bounded T406 admission target. |
| Stop Conditions | Stop if the candidate would make a physical/L3 claim, imports a reference implementation, lacks a single owner, or bypasses a frozen profile boundary. |
| Exit Criteria | A single eligible DeskPro fallback candidate is in Queue before Generic PC, linked to an approved proposal with a finite receiver ledger and a bounded first implementation admission. |
| Original Owner Request | Implement DeskPro 386 L3-level timing and all hardware gaps: original sources first, reference implementations second, generic IBM AT skeletons if necessary; label all three levels and keep moving. |
| Similar-Issue Sweep | Governance-only: inspect Queue, T398/T399/T403/T405 histories and TODO receivers for any other DeskPro row stranded behind an ineligible candidate; no runtime defect class is repaired. |

## Current Technical Baseline

- **Current developer artifact:** T405 S1 P1 `vm-0-5-0405` /
  `build/output/nxvm_0_5_0405.exe`, SHA-256
  `6A2234EBBD08910686B6A9EB708DBCE3269DFA0379CB9C89ABF3CB9230FC8E48`.
  [T405 x86 GCC evidence](../etc/evidence/t405-s1-x86-gcc-host-build-compatibility.md) records 286/286 current-gate passes on both x86 and x64 hosts; this remains no L3 claim.
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
| T405 | Closed: x86 GCC host build accepts the current product after pointer-width and validated-media-offset repairs; x86 and x64 current gates both pass 286/286, with no L3 claim. [Evidence](../etc/evidence/t405-s1-x86-gcc-host-build-compatibility.md). |
| T404 | Closed: every frozen public profile/catalog/media/controller/display/input route has direct functional proof or a named physical receiver; no timing or L3 claim. [Closure audit](../etc/evidence/t404-s6-batch-b-functional-reconciliation.md). |
| T403 | Closed: shared keyboard serial endpoint prevents accepted break loss under full 8042 output; functional KBC flow is complete while physical keyboard/8042/DeskPro timing remains transferred. [Closure audit](../etc/evidence/t403-s2-keyboard-flow-control-closure-audit.md). |
| T402 | Closed: D3PE-first functional D4 matrix accepted; the Model-40 control aperture repair is reference-labelled where primary material is incomplete, while cache/DRAM/ISA physical timing and L3 remain transferred. [Closure audit](../etc/evidence/t402-s2-d4-functional-closure-audit.md). |
| T401 | Closed: bounded four-profile CPU semantic/source-timing audit; every frozen form is proven, repaired, nonphysical-observed, rejected or transferred, without an x87, physical CPU, board or L3 claim. [Closure audit](../etc/evidence/t401-s78-task-closure-audit.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
