# Project Status

## Current Work

## M5 T468 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T468 S2, one-session coordinator, executor and closure review. |
| Admission And Approval | S1's accepted 15-row ledger authorizes its implementation-gap audit. S2 may classify current behavior and name a sole repair owner; it may not implement, select a controller/device/media personality, or promote resemblance to ATA conformance. |
| Objective | Trace every ATA-R1--R5, ATA-F1--F5 and ATA-T1--T5 row through current HDC, logical media, PIC, scheduler, profile and focused tests; freeze Checklist 2 with behavior, proof, gap, exact owner and disposition. |
| Non-goals | No runtime, public ABI, CMake, profile, fixture, firmware, guest-media, external source or artifact change. |
| Reference Baseline | [T468 S1 Checklist](../etc/research/t468-s1-ata3-function-timing-checklist.md) and [T450 S18 audit](../etc/evidence/t450-s18-hdc-ata-code-gap-audit.md). Current code remains evidence only. |
| Candidate Proposal | [M5 Core HDC ATA Phase Contract](../proposals/m5-core-hdc-ata-phase-contract.md), S2 implementation-gap audit. |
| Files And ABI Surface | Checklist 2, history/status and evidence/index only. Inspect `hdc.c`, its public contracts, Core machine/media/PIC/scheduler/profile paths and relevant tests read-only. |
| Applicable Rules | Source/research, architecture, coding, execution and documentation rules: one HDC/media/PIC/scheduler owner path; manual is normative; external models do not choose a personality; L2 only for a genuinely absent sourced input/receiver. |
| Verification | Exhaustive writer/consumer search, row-by-row source trace, focused-test inventory, no-runtime-change proof, documentation governance and diff check. |
| Expected Markers | One 15-row Checklist 2; every gap is either an S3/S4 repair through an existing owner or an exact L2 transfer; no duplicate state, profile mutation, generic setter or host bridge. |
| Asset Needs | Existing repository and already-admitted read-only sources only; no asset import. |
| Reporting Requirements | Record exact paths/tests, behavior and absence separately, selected-current versus capable receiver status, each repair owner and every retained L2 boundary. |
| Stop Conditions | Stop for a required personality decision, a conflict with ATA-3, a missing owner that would require architectural expansion, or any implementation work. |
| Exit Criteria | Checklist 2 has all 15 IDs once; S3/S4 scope is finite, source-backed and carries no hidden controller/device/media selection. |
| Original Owner Request | Complete the remaining controller tasks with manual authority, cross-model verification, exact L2 boundaries and minimalist single-owner design. |
| Similar-Issue Sweep | Search all HDC protocol branches, media bindings, IRQ publication sites, profile construction inputs, port accessors, reset/detach paths and focused tests for duplicated state, MFM aliasing or unqualified timing claims. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0467`; the stripped Release
  `nxvm_0_5_0467.exe` has SHA-256
  `CD99B2E1A0F0AC0FB8982838BEC56237C29C1DA24BD9E5E3B608117B20EAF6C1`.
  Debug remains the current-gate route. T434 has one copied Core timing-plan
  publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB
  session composition.
  T386 closes selected-device functional completeness at S29; its retained
  [closure audit](../etc/evidence/t386-s29-functional-closure-audit.md) fixes
  HDC current-gate coverage and transfers board, firmware and physical work.
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. This is only the
  selected Model 339 configuration and preserves every documented physical,
  fixed-disk, generic-PC/AT and later-machine boundary.
- **T285 display implementation:** `INT 10h` modes `0Dh`, `0Eh` and `10h`
  reach the VADP-owned planar frame path through real controller ports; the
  copied-frame consumer boundary has no BIOS mode-state dependency.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T468 S1 | Accepted: the ATA/HDC Checklist 1 retains all 15 inherited row IDs. Rendered ATA-3 is normative; 86Box, Bochs, QEMU and MAME are bounded corroborators, while PCjs is inapplicable. No current HDC personality is presumed ATA. Manual interface facts are L3; only a source-qualified value consumed through the existing sole HDC path can become Board L3. [Checklist](../etc/research/t468-s1-ata3-function-timing-checklist.md) and [history](../history/M5-T468-core-hdc-ata-phase-contract.md). |
| T467 | Closed: all 17 selected IBM-CGA rows reconcile to one VADP/Core/profile/snapshot path. Manual-L3 digital behavior and bounded Model-339 Other-L3 cadence are complete; only exact 160x100, default/ISA-wait timing, light-pen lifecycle and physical-monitor boundaries remain L2. Serial current-gate passes 295/295; stripped Release 0467 is recorded in [history](../history/M5-T467-core-ibm-cga-completeness.md). |
| T466 | Closed: Manual-L3 D/E/10 geometry reaches the single VADP state owner through real firmware and guest port writes. The profile declaration, cold-start/text routing and all fixtures now agree; unknown planar geometry falls through existing legacy selection rather than inventing a frame. The stripped Release 0466 artifact and its hash are recorded in [history](../history/M5-T466-core-vadp-phase-contract.md). |
| T465 | Closed: selected Intel 8272A source reconciliation, command/reset repair and per-drive parallel Seek close the logical controller contract; physical media and board-time conversion remain explicit L2. Release 0465 is stripped and the 294/294 serial gate is recorded in [history](../history/M5-T465-core-intel-8272a-logical-media-phase-contract.md). |
| T464 | Closed: all sixteen KBC rows are Manual L3 or explicit L2/L4. One Core owner now covers selected command/input/test state; 294/294 serial current-gate tests pass and stripped Release 0464 is recorded in history. |
| T463 | Closed: the 16-row MC146818A/IBM AT ledger has one Core phase owner, a copied L3/L2 board timing plan, explicit L2/L4 boundaries and focused proof. The full current gate ran 294/294; stripped Release 0463 is recorded in [history](../history/M5-T463-core-rtc-cmos-phase-contract.md). |
| T462 | Closed: one immutable copied plan carries only qualified Model-339 PIT/DMA selections; PIC remains explicit L2. The seven corrected PIT-transition regressions, 294/294 serial current gate, 77/77 specialized gates, documentation governance and stripped 0462 artifact complete the controller-board contract. [History](../history/M5-T462-core-controller-board-timing-contract.md). |
| T461 | Closed: one Core PIT owner covers manual P1--P14 and one `OUT0 -> IRQ0 -> PIC refresh` route closes P15; P16--P18 remain explicit L2. Full current-gate is 294/294 and its stripped artifact is retained in history. [History](../history/M5-T461-core-pit-8254-phase-contract.md). |
| T460 | Closed: all 16 DMA rows are disposed through the sole `dma.c` owner; normal/compressed and M2M service phases have focused proof, the five-clock conversion remains explicit L2, serial current-gate is 294/294, and the stripped 0460 artifact is recorded. [History](../history/M5-T460-core-dma-8237a-phase-contract.md). |

## Recent Governance

- **M5 Td S147 P3:** made result simplicity enforceable: a correct repair must
  consolidate duplicate paths, mirrored state, obsolete ownership, and avoidable
  reverse dependencies when the admitted mechanism permits it. Closure records
  the retained path and owner; forwarding wrappers and parallel compatibility
  paths do not qualify as simplification. No runtime, API, or artifact changed.

- **M5 Td S147 P2:** reconciled current support terminology (`states/CURRENT.md`,
  `type_status`, and completed T461/T462 evidence retention) and renamed the
  DMA-local reset helper without changing behavior, ABI, artifact, or ownership.
  Documentation governance, dependency, C-facade, and Debug `core-machine`
  rebuild checks pass.

- **M5 Td S146 P1:** refined the queue-first controller board-timing candidate
  into construction-only typed values, retained dynamic device inputs, and
  bounded S1--S4 ownership. It prohibits live timing setters and preserves L2
  provenance after deterministic ratio normalization. No runtime or artifact
  changed.

- **M5 Td S145 P1:** placed the Core controller board-timing-contract candidate
  first in Queue. Its proposal freezes the PIC/DMA/PIT L2 exception ledger and
  requires one copied profile-value-to-Core-owner path; exact arithmetic from
  an L2 premise remains L2 board provenance. No runtime or artifact changed.

- **M5 Td S144 P1:** governed all 16 8237A source and code-gap rows with
  `Manual L3`, bounded `Other L3` corroboration and explicit `fallback to L2`
  boundaries. Intel/IBM remains normative; emulator behavior neither overrides
  it nor creates a Core claim. DMA implementation, artifact and Queue order
  are unchanged; documentation governance passes.

- **M5 Td S143 P1:** made the eighth Core L3 integration candidate the sole
  planned receiver for host-paced guest-time synchronization: Core owns
  deadline composition and guest progression; VM only paces completed guest
  progress. The duplicate TODO is removed, T459 evidence links to the receiver,
  queue order and runtime behavior are unchanged, and documentation governance
  passes.

- **M5 Td S142 P1:** added the single execution rule that current product
  artifacts are stripped Release builds without compiler debug information;
  admitted runtime debuggers remain product functionality, while Debug builds
  are development/gate-only. Documentation governance passes; no code or
  artifact changed.

- **M5 Td S141 P1:** recorded three deferred, source-gated controller
  admissions: XT/AT PIC topology, later-AT ELCR/PCI routing, and APIC/IOAPIC.
  Core owns every guest-visible controller state; profile only selects an
  immutable topology. No queue, source or runtime behavior changed.

- **M5 Td S140 P1:** added and ordered first the bounded request-bridge
  full-build restoration and parallel Console profile-smoke isolation
  candidates; no runtime behavior changed.

- **M5 Td S139 P1:** inserted one source-admission and implementation-gap
  inventory candidate ahead of the eight controller implementations; it
  requires two finite checklists per controller and makes no source or runtime
  change. [Evidence](../etc/evidence/td-s139-controller-manual-inventory-admission.md).

- **M5 Td S138 P1:** replaced the aggregate controller/device implementation
  candidate with ordered PIC, DMA, PIT, RTC, KBC, FDC/media, VADP and
  source-blocked HDC proposals; the aggregate now supplies shared context only.
  [Evidence](../etc/evidence/td-s138-controller-phase-program-decomposition.md).

- **M5 Td S137 P1:** clarified the queued CPU-to-board transaction candidate's
  sole-owner data flow, direct-path removal constraint, L2/rejection boundary,
  and bounded S1--S6 implementation sequence; no runtime behavior or Queue
  order changed. [Evidence](../etc/evidence/td-s137-core-transaction-proposal-clarification.md).

- **M5 Td S136 P1:** independently rebuilt the complete Td S135 revision with
  WinLibs UCRT GCC and Ninja (108/108), recorded its output hash, and removed
  the temporary verification tree. [Evidence](../etc/evidence/td-s136-t447-compile-verification.md).

- **M5 Td S135 P1:** removed the two unnecessary VM-to-Core device
  implementation includes and completed T447's history/proposal/status
  migration. [Evidence](../etc/evidence/td-s135-t447-closure-reconciliation.md).

- **M5 Td S134 P1:** created the coverage-bearing architecture-boundary debt
  closure proposal, which admitted and closed as T447; its retained companion
  is now in [T447 history](../history/M5-T447-architecture-boundary-debt-closure.md).

- **M5 Td S133 P1:** audited actual T438--T443 owner paths, focused regressions,
  artifacts, commits and topology; it archived the two retained completed
  proposals and transferred T344 plus 20 current-fast failures to the queued
  current-gate restoration candidate. [Evidence](../etc/evidence/td-s133-recent-task-closure-audit.md).

- **M5 Td S132 P1:** republished the generated T435 S5 8086 result artifact
  after current Core-private source-form IDs evolved. The runner and both
  8086 result/decoder gates pass; all 1,053 records retain identical
  non-form-ID observations. [Evidence](../etc/evidence/td-s132-8086-derived-result-reconciliation.md).

- **M5 Td S131 P1:** admitted nine bounded repair candidates ahead of future
  M5 capability work, promoted firmware materialization from TODO, and refined
  the queued YAML candidate's grammar/selection acceptance surface. The
  documentation-governance gate passes; this changes no runtime behavior.

- **M5 Td S130 P1:** sixth-pass manual review of selected VM production
  dependencies on Core-private machine headers confirms a keyboard scan-set
  contract gap and five unused include residues. The indexed
  [evidence](../etc/evidence/td-s130-sixth-pass-audit.md) records exact paths,
  bounded non-findings, and the existing TODO transfer; this is not a
  whole-repository completion or runtime-correctness claim.

- **M5 Td S129 P1:** fifth-pass manual review of Core product-session payloads,
  selected machine state headers, debug/execution providers, and utilities
  confirms raw mutable option/command storage crosses the public session
  boundary. The indexed [evidence](../etc/evidence/td-s129-fifth-pass-audit.md)
  records bounded non-findings and remaining scope; the repair extends the
  existing session-manager boundary task rather than duplicating it. This is
  not a whole-repository completion or runtime-correctness claim.

- **M5 Td S128 P1:** fourth-pass manual review of VM composition, profile,
  product, and public-contract implementation confirms mutable VM-product
  console/catalog state escapes, a test-only duplicated Model-40 construction
  path, and a composition-to-product reverse adapter dependency. The indexed
  [evidence](../etc/evidence/td-s128-fourth-pass-audit.md) also records bounded
  lifecycle/profile-direction non-findings and remaining scope; all repairs
  extend existing non-overlapping TODO admissions. This is not a
  whole-repository completion or runtime-correctness claim.

- **M5 Td S127 P1:** third-pass manual review of CMake platform/source
  ownership, duplicate product command authority, and Core/VM test boundaries
  confirms four additional concrete findings, including systemic VM test
  access to embedded Core state. Named allocator-failure source builds are
  separately confirmed as narrow, recorded test instrumentation rather than a
  new production path. The indexed
  [evidence](../etc/evidence/td-s127-third-pass-audit.md) retains exact
  paths, transfers, and remaining audit limit; this is not a whole-repository
  completion or runtime-correctness claim.

- **M5 Td S126 P1:** second-pass manual review of Core stateful contracts,
  collaborator and plan endpoints, Core debugger context, VM/VDM equivalent
  headers, and host resource/input paths records five additional concrete
  architecture/coding findings. The indexed
  [evidence](../etc/evidence/td-s126-second-pass-audit.md) records the exact
  source paths, one positive input-route disposition, and remaining scope; all
  repairs are transferred to TODO. This is not a whole-repository completion or
  runtime-correctness claim.

- **M5 Td S125 P1:** manually audited the current authorities, source owners,
  composition paths, public contracts, platform adapters, VDM skeleton, tests,
  CMake declarations, and corroborating structural gates. The indexed
  [audit evidence](../etc/evidence/td-s125-manual-architecture-coding-audit.md)
  records nine concrete architecture/coding findings and explicit non-findings;
  all repair work is bounded in TODO. This is not a runtime-correctness claim.

- **M5 Td S124 P1:** ran limited static dependency and C-facade checks. Those checks passed, but they were not a full manual architecture/code audit and do not establish absence of invariant violations; Td S125 performs the required source and documentation review.
