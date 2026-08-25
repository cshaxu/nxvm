# Project Status

## Current Work

## M5 Td S148 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance |
| Admission And Approval | Owner request in this turn authorizes a full re-audit of PIC, DMA, PIT, RTC-CMOS, KBC, Intel 8272A FDC, VADP CGA/EGA and ATA/HDC against the stated L3/L2/L1/L0 rule. |
| Objective | Reclassify every existing controller checklist row by whether its evidence is a direct manual/model/board L3 fact, an external L3 input receiver, estimated L2 timing, L1 logical ordering, or L0 missing ordering. |
| Non-goals | No controller feature, timing number, profile selection, API, source import, artifact or runtime-code change unless a concrete audit contradiction requires a separately admitted repair. |
| Reference Baseline | T460--T468 controller histories; their Checklist 1/2 evidence; T462 board-contract record; T467 CGA and T468 ATA ledgers; current architecture and source policy. |
| Candidate Proposal | The completed controller phase records are audit subjects; Td S148 creates no new implementation proposal. |
| Files And ABI Surface | `docs/states/CURRENT.md`, one indexed Td evidence record and only the existing checklist/evidence records whose classification wording must change; no ABI. |
| Applicable Rules | Manual authority remains normative; external emulator behavior is a separately labelled model L3; a copied, externally supplied board term consumed by its sole owner is Board L3; unproved internal estimates are L2; logical-only order is L1; absent order is L0; no L4 category is used. |
| Verification | Enumerate every finite row across the eight controllers and separate VADP CGA/EGA modes; verify one source/owner/proof/disposition per row; run documentation governance and inspect the actual documentation diff. |
| Expected Markers | Every prior generic L2 is either promoted to a qualified L3 receiver or explicitly retained as L2/L1/L0 with a reason; no row silently becomes L3 merely by arithmetic. |
| Asset Needs | Existing archived manuals, read-only external-model findings and prior evidence only; no new source import or firmware/media acquisition. |
| Reporting Requirements | Record the taxonomy, row counts per controller, every promotion/demotion, retained non-L3 reason, affected owner and any separately required code repair. |
| Stop Conditions | Stop for missing authoritative evidence that cannot be classified from the existing ledger, or for a discovered runtime defect needing new implementation authority. |
| Exit Criteria | All eight controller ledgers use the requested hierarchy consistently, VADP CGA and EGA are separate, all changed records are indexed, and documentation governance passes. |
| Original Owner Request | Treat external L3 inputs as L3; reserve L2 for internal proportional estimates, L1 for logic-only order and L0 for absent order; re-audit all eight controllers including VADP CGA/EGA. |
| Similar-Issue Sweep | Search every controller Checklist 1/2, board-timing contract, history closure and current-summary L-level statement for stale generic `L2`, `L4` or ambiguous receiver wording. |
| Closure | Accepted: the indexed Td S148 evidence covers all 145 frozen rows, separates VADP CGA/EGA, records every receiver/value distinction and residual L2/L1/L0 reason, and documentation governance passes. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0468`; the stripped Release
  `nxvm_0_5_0468.exe` has SHA-256
  `99100D6B99ABD98FF584F6B4179B6BC8B96519B53F234FBE4CC45DDBAC860BF5`.
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
| T468 | Closed: the ATA-3 15-row task-file/media contract has one HDC owner, one media owner and one PIC route; the serial current gate passes 295/295 and stripped Release 0468 is recorded in [history](../history/M5-T468-core-hdc-ata-phase-contract.md). |
| T467 | Closed: all 17 selected IBM-CGA rows reconcile to one VADP/Core/profile/snapshot path. Source and model facts remain retained; their current L3/L2/L1/L0 interpretation is [Td S148](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md). Serial current-gate passes 295/295; stripped Release 0467 is recorded in [history](../history/M5-T467-core-ibm-cga-completeness.md). |
| T466 | Closed: Manual-L3 D/E/10 geometry reaches the single VADP state owner through real firmware and guest port writes. The profile declaration, cold-start/text routing and all fixtures now agree; unknown planar geometry falls through existing legacy selection rather than inventing a frame. The stripped Release 0466 artifact and its hash are recorded in [history](../history/M5-T466-core-vadp-phase-contract.md). |
| T465 | Closed: selected Intel 8272A source reconciliation, command/reset repair and per-drive parallel Seek close the logical controller contract. The retained physical/media/time boundaries use the current [Td S148](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md) classification. Release 0465 is stripped and the 294/294 serial gate is recorded in [history](../history/M5-T465-core-intel-8272a-logical-media-phase-contract.md). |
| T464 | Closed: all sixteen KBC rows retain their selected command/input/test owner. Their current source/input/estimate/order classification is [Td S148](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md); 294/294 serial current-gate tests pass and stripped Release 0464 is recorded in history. |
| T463 | Closed: the 16-row MC146818A/IBM AT ledger has one Core phase owner and copied board input plan. Its current L3/L2/L1/L0 interpretation is [Td S148](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md). The full current gate ran 294/294; stripped Release 0463 is recorded in [history](../history/M5-T463-core-rtc-cmos-phase-contract.md). |
| T462 | Closed: one immutable copied plan carries the qualified Model-339 PIT/DMA selections; the current receiver/value distinction and PIC residual are in [Td S148](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md). The seven corrected PIT-transition regressions, 294/294 serial current gate, 77/77 specialized gates, documentation governance and stripped 0462 artifact complete the controller-board contract. [History](../history/M5-T462-core-controller-board-timing-contract.md). |
| T461 | Closed: one Core PIT owner covers manual P1--P14 and one `OUT0 -> IRQ0 -> PIC refresh` route closes P15. Its retained boundaries are reclassified by [Td S148](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md). Full current-gate is 294/294 and its stripped artifact is retained in history. [History](../history/M5-T461-core-pit-8254-phase-contract.md). |

## Recent Governance

- **M5 Td S148 (active):** re-audits all 145 frozen PIC, DMA, PIT, RTC, KBC,
  8272A FDC, VADP EGA/CGA and ATA/HDC rows under the owner-approved hierarchy:
  direct manual/model facts and real sole-owner external inputs are L3; only
  internal estimates are L2; causal-only relations are L1; absent Core logic
  is L0.  The [row-complete evidence](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md)
  supersedes earlier L-level vocabulary without rewriting historical source
  records.

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
