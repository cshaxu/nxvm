# Project Status

## Current Work

**Active.** T429 remains open between accepted subtasks.
## M5 T429 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user; standing approval in this task thread (2026-08-18) for continued DeskPro L3 work, commits, pushes, and pragmatic generic-AT mechanisms where original/reference behavior is unavailable. Scope: repair the explicit D4-RAM versus CECG-aperture classification boundary; no firmware/media or third-party import. |
| Objective | Make Core external memory-page timing eligible only for declared profile memory ranges, then bind Model-40 D4 conventional RAM explicitly and prove the CECG A0000h aperture does not inherit that D4 policy. |
| Non-goals | No CECG-specific aperture wait scalar, NOWS/BUSRDY/IOCHRDY claim, RAM-controller physical model, D4 PAL inference, raster/monitor behavior, new transaction/scheduler path, or L3 acceptance. |
| Reference Baseline | `master` at `9698677e`; accepted T429 S1 port-space lifecycle at `1c02a853`; T386 S28 owns CECG aperture/page functional behavior. |
| Candidate Proposal | [DeskPro 386 physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md), receiver 1; S2 consumes the generic-AT D4/CECG memory-classification boundary. |
| Files And ABI Surface | Core external-cycle timing configuration/receiver, Model-40 composition, focused Core/Model-40 regression, evidence/index, status/proposal/history/queue. |
| Applicable Rules | Core owns timing eligibility and lifecycle; VM binds only Model-40 ranges. C11/type vocabulary and focused tests apply. Source policy prohibits retained sources/assets. Execution/documentation rules require evidence, sweep, actual-diff review, and immediate push. |
| Verification | Focused Core and Model-40 CTest; 5170 composition isolation; diff check; documentation gate; serial current-gate before closure. |
| Expected Markers | New `M5:T429:S2:D4-CECG-MEMORY-CLASS:OK`; existing T429 S1, Model-40 CECG and 5170 markers remain green. |
| Asset Needs | None. |
| Reporting Requirements | Report generic-AT classification tier, exact D4/CECG boundary, Core/Profile ownership, tests, pushed commit, and residual aperture wait/physical transfer. |
| Stop Conditions | Stop and transfer if a range requires vendor firmware, CECG electrical behavior, physical scalar calibration, a second transaction/scheduler, or a non-Model-40 profile change. |
| Exit Criteria | Page timing has one explicit eligible-memory selection, Model-40 selects D4 conventional RAM only, CECG aperture is proved outside it, 5170 retains prior behavior, cancellation/reset remain correct, and gates pass. |
| Original Owner Request | User directed continued DeskPro L3 implementation using original/reference/generic tiers pragmatically while preserving Core/VM boundaries and repairing confirmed issues. |
| Similar-Issue Sweep | Search every external-cycle timing config and provider call, all profile bindings, VADP apertures, D4 mappings, reset/cancel paths, and 5170 composition; classify every range change. |
## Current Technical Baseline

- **Current developer artifact:** T421 S1 `vm-0-5-0419` / `build/output/nxvm_0_5_0419.exe`, SHA-256 `859E1B93C6891E8EAAF0D98D4DBEF25F2383F911EC243390A50FB9A9CDBBA5BF`. T421 adds one Core-owned logical port-61h/PIT-channel-2 speaker line: Model-40 D4 and IBM 5170 planar configuration select it without a second provider; host audio and physical acoustic behavior remain transferred.
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
| T429 S1 | Accepted: generic-AT CECG 8-bit port-wait skeleton at `1c02a853`; Core owns the memory-versus-port lifecycle and committed wait, Model-40 selects documented ports, and 5170 remains unselected. [Evidence](../etc/evidence/t429-s1-cecg-8bit-bus-wait.md). |
| T428 | Closed: Model-40 selects a generic-AT Core prefetch reservation lifecycle with reset/HOLD/refresh priority; it publishes no physical cycle, page hit or timing result. [Evidence](../etc/evidence/t428-s1-generic-prefetch-reservation.md). |
| T427 | Closed: original-source CPU BUSRDY gate holds the existing Core external completion wait without retirement; Model-40 selects it and 5170 is isolated. [Evidence](../etc/evidence/t427-s1-deskpro-cpu-busready-gate.md). |
| T426 | Closed: independent audit retains all tier-labelled logical receivers but rejects DeskPro physical/L3 acceptance pending named source/corpus blockers. [Evidence](../etc/evidence/t426-s1-deskpro-physical-l3-reaudit.md). |
| T425 | Closed: reconciled FDC/HDC/KBC/DMA/PIC/NMI logical device phases at declared tiers; physical rates and propagation remain transferred. [Evidence](../etc/evidence/t425-s1-deskpro-device-phase-reconciliation.md). |
| T424 | Closed: VM Model-40 BYOB now proves synthetic manifest validation, immutable mapped reset-vector execution and cold-reset replay; vendor POST/service/timing corpus remains external. [Evidence](../etc/evidence/t424-s1-model40-byob-reset-lifecycle.md). |
| T423 | Closed: chargeable D4 external CPU-memory waits now advance through a Core-owned non-retiring completion state before retirement; BWAIT/ISA/async-prefetch physical work remains transferred. [Evidence](../etc/evidence/t423-s1-cpu-board-retirement-wait.md). |
| T422 | Closed: original D3PE SLOWD*/refresh/auxiliary-PIT channel-2 bridge uses one Core hold route; Model-40 binds it and 5170 remains isolated. [Evidence](../etc/evidence/t422-s1-d4-slowdown-hold.md). |
| T421 | Closed: Core owns the logical port-61h/PIT-channel-2 speaker line; S2 proves existing planar/D4 port-B exclusivity in both orders. Model-40 and 5170 keep one selected route. [S1](../etc/evidence/t421-s1-d4-speaker-line.md), [S2](../etc/evidence/t421-s2-port-b-exclusivity.md). |

## Recent Governance

- **M5 Td S107 P1:** retired the completed tiered-fallback foundation from the active Queue, promoted the DeskPro physical-cycle program, and ordered CPU-to-board timing, device phases, Compaq fixed disk, CECG, BYOB lifecycle, and independent L3 re-audit with retained evidence tiers.

- **M5 Td S106 P1:** corrected the retained PCjs DeskPro HDC configuration fact to two generic AT Type-1 10.16 MB drives; the Compaq WD 40 MB non-qualification and all physical receivers remain unchanged.

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
