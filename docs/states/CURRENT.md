# Project Status

## Current Work

**Active: M5 T429 S1.**

## M5 T429 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner: user; standing approval in this task thread (2026-08-18) to continue DeskPro L3 work, commit, and push to `master`. Scope: one generic-AT CECG 8-bit expansion-bus wait skeleton, with original sources retained only for CECG addressing and 8-bit interface facts; no third-party import or firmware/media retention. |
| Objective | Give Core one profile-selected, bounded external 8-bit CECG bus-cycle wait classification that advances non-retiring source-cycle quanta before the existing retirement route; bind the selected Model-40 CECG register/aperture surface without a VM transaction or scheduler owner. |
| Non-goals | No claim that CECG asserts or deasserts NOWS/ BUSRDY; no waveform, BCLK-to-host calibration, asynchronous prefetch, D4 PAL/bank inference, new CPU/DMA path, or physical/L3 acceptance. |
| Reference Baseline | `master` at `c0b096b5`; T423 Core completion wait, T427 CPU BUSRDY gate, and T428 reservation are retained inputs. Original Compaq DeskPro 386 Volume I and CECG technical reference were inspected transiently under the source policy; no copies are retained. |
| Candidate Proposal | [DeskPro 386 physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md), receiver 1. Ledger batch: generic-AT selected CECG memory/I/O wait skeleton; original addressing/width facts only. |
| Files And ABI Surface | `src/core/machine/machine_interface.h`, `src/core/machine/machine.[ch]`, Model-40 composition/profile binding, focused Core/VM regression, proposal, evidence/index, history/status/queue. Public surface only gains the minimum copied timing configuration required by Core. |
| Applicable Rules | `docs/design/ARCHITECTURE.md`: Core owns shared CPU transaction and availability/wait rules; VM binds only board difference. `docs/rules/ARCHITECTURE.md`: one owner/production path and no reverse dependency. `docs/design/CODING.md` and `docs/rules/CODING.md`: module naming, C11/type vocabulary, focused tests. Source policy: reference facts reduced to neutral statements; no retained source/firmware/media. Documentation and execution rules: active packet, evidence, actual-diff review, immediate push, closure P. |
| Verification | Build focused target(s); run their CTest labels; `git diff --check`; documentation governance gate; full serial `current-gate` CTest before closure. |
| Expected Markers | New `M5:T429:S1:CECG-8BIT-BUS-WAIT:OK`; existing Model-40/5170 composition markers remain green. |
| Asset Needs | None. Original public technical references are transient research only; no asset, ROM, PDF, OCR text, hash, local source path, or third-party code enters the repository. |
| Reporting Requirements | Report source tier, selected surface, Core/Profile ownership, regression, push commit, and residual NOWS/BUSRDY/physical-clock transfer. |
| Stop Conditions | Stop and transfer if implementation requires a retained third-party source, a CECG-specific NOWS assertion/deassertion, electrical waveform/calibration, unselected expansion hardware, or a second transaction/scheduler path. |
| Exit Criteria | Core owns one bounded classified wait route, Model-40 selects only its documented CECG surface, 5170 remains unselected, reset/cancellation/retirement proof is focused, evidence labels the generic-AT wait and original addressing/width boundary, and all required gates pass. |
| Original Owner Request | User directed immediate continued DeskPro L3 implementation, with original/reference/generic evidence tiers, pragmatic execution, preserved Core/VM boundaries, and repair of confirmed in-scope issues. |
| Similar-Issue Sweep | Defect class: port/memory external-cycle waits must not be globally charged or require a second CPU transaction. Search all Core CPU transaction hooks, profile composition configs, Model-40 and 5170 bindings, tests, proposal/evidence records. Verify every selected Model-40/5170 configuration and cold reset. |

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
