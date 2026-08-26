# Project Status

## Current Work

**Open: M5 T487 S1.**

## M5 T487 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner-approved queue execution on 2026-08-26 and Td S153 complete-unit rule: the next independent unit is the IBM 5160 board/bus contract. |
| Objective | Establish the original IBM 5160 board/bus source ledger, including visually verified page authority, OCR condition, formulas and any lawful reference-emulator cross-check boundary. |
| Non-goals | No production code, timing selection, CPU instruction work, controller implementation, ROM/firmware import, guest media, profile/runtime change or inferred timing value. |
| Reference Baseline | Closed T486, queued IBM 5160 board/bus proposal, source policy and existing external assets. |
| Candidate Proposal | [IBM 5160 8088 board, memory, ROM and ISA phase contract](../proposals/m5-xt-8088-board-bus-phase.md). |
| Files And ABI Surface | Source-ledger evidence, evidence index, history and Current only; inspected assets remain external and no ABI changes. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/etc/operations/policy/source-policy.md`, `docs/design/ARCHITECTURE.md`, `docs/rules/ARCHITECTURE.md`. Core owns timing/transactions; research never imports source or firmware. |
| Verification | Locate and visually inspect original IBM source pages covering oscillator, bus cycle, RAM/ROM, reset, ISA and NMI-facing facts; distinguish source fact, OCR uncertainty, unavailable fact and external-emulator corroboration. |
| Expected Markers | One finite page/formula source ledger suitable for List 1, with every unavailable or ambiguous item explicitly marked before implementation planning. |
| Asset Needs | Lawful external owner-managed documentation only; no firmware image, Microsoft binary, guest media or third-party source import. |
| Reporting Requirements | Record asset identity, original/source quality, visual page checks, topic coverage, source confidence and reference cross-check limits. |
| Stop Conditions | Stop if the original board source cannot be lawfully located, page authority is ambiguous, or a required fact exists only in a forbidden asset; record the gap rather than use an emulator as primary authority. |
| Exit Criteria | Every intended board List-1 topic has an original-source page/formula or an explicit absence/ambiguity finding, and any external implementation is bounded to corroboration. |
| Original Owner Request | For every newly admitted chip/controller/board unit, start with original material and a complete source/function/timing gap process; do not begin opportunistic repairs. |
| Similar-Issue Sweep | Search IBM 5160/PC-XT system-board, oscillator, ISA, RAM/ROM, reset, NMI and timing references across the external manual archive and existing repository provenance. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0484`; the stripped Release
  `nxvm_0_5_0484.exe` has SHA-256
  `57FC91744B3C0F0F123D51D35E34423D271A965DA1D4A3BDC62AFDF66914B280`.
  Debug remains the current-gate route. T471 preserves Core-owned progression:
  a verified axis is Standard-paced only by host waiting against completed
  Core progress. T472 extends that comparison to an explicit L2 macro axis,
  removes the fixed HLT backoff, and retains unqualified profiles as no-wait.
  T434 has one copied Core timing-plan
  publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB
  session composition.
  T386 closes selected-device functional completeness at S29; its retained
  [closure audit](../etc/evidence/t386-s29-functional-closure-audit.md) fixes
  HDC current-gate coverage and transfers board, firmware and physical work.
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. T479 S5 has
  replaced the false no-fixed-disk declaration with its one IBM Type-3
  WD1003/ST-506 personality. The normal logical CHS/PIO path and exact
  step-selector state are present; physical MFM/ECC/mechanical/parity behavior
  remains explicitly outside this route.
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
| T486 | Closed: the complete 8088 manual source/List-1/List-2/one-owner implementation chain closes without a parallel timing path; Group-3, WAIT and non-string-prefix source boundaries remain explicit. [Closure audit](../etc/evidence/t486-s5-8088-closure-audit.md). |
| T485 | Withdrawn, not completed: accepted S1--S20 evidence remains retained, but Td S152 transfers the uncompleted aggregate XT scope to independent CPU/board/controller candidates. [Record](../etc/evidence/td-s152-unit-task-admission-governance.md). |
| T484 | Closed: IBM 5160-268 now has one immutable 8088/profile plan, XT PPI/FDC/CGA/Xebec functional owners and one typed external-ROM BYOB product route; all remaining board/device timing transfers to T485. [Closure audit](../etc/evidence/t484-s22-xt-functional-closure-audit.md). |
| T482 | Closed: one strict root YAML request reaches the sole `SESSION OPEN` authority and resolver without CLI re-encoding; 296/296 current-gate tests and stripped Release 0480 pass. [Closure audit](../etc/evidence/t482-s4-closure-audit.md). |
| T483 | Closed: the IBM 5160-268 BOM, complete current-owner capability matrix and Manual-L3/unresolved timing partition establish one functional and one timing receiver without implying an XT runtime path. [Closure evidence](../etc/evidence/t483-s3-xt-timing-source-partition.md). |
| T481 | Closed: one global CPU/controller/Core-time/profile/VM audit leaves no unrecorded duplicate owner, host-to-Core tick injection or false capability claim; residuals have one ordered receiver. [Closure audit](../etc/evidence/t481-s5-global-reconciliation.md). |
| T480 | Closed: VADP remains the sole guest-video owner; IBM VGA DAC, chain-4 and Mode-13 frame capability is source-backed but unselected by every current profile/card. The 296-target gate and stripped Release 0480 pass. [Closure audit](../etc/evidence/t480-s5-profile-closure-audit.md). |
| T479 | Closed: one Core HDC/media boundary retains explicit ATA, Compaq and IBM WD1003 personalities; seven focused regressions, the 294-target gate and stripped Release 0479 hash pass. IBM 5160/Xebec and WD1007A-WAH ESDI contracts transfer only to their missing immutable profile receivers. [Closure audit](../etc/evidence/t479-s8-closure-audit.md). |

## Recent Governance

- **M5 Td S153 P1:** hardened the independent hardware-unit T rule across the
  active 8088 work and XT Queue: original source, complete List 1, complete
  List 2, then a single owner-local implementation batch. Final audits can
  transfer but never repair. The [evidence](../etc/evidence/td-s153-unit-task-queue-hardening.md)
  records all eleven candidates. No runtime, ABI, build, artifact or source
  material changed.

- **M5 Td S152 P1:** withdrew T485's uncompleted aggregate scope without a
  false closure and replaced it with per-unit 8088/XT candidates. Every new
  hardware T must complete original-source, List-1, List-2 and one-owner
  implementation stages before it may repair code. The [evidence](../etc/evidence/td-s152-unit-task-admission-governance.md)
  records the transfer. No runtime, ABI, build or artifact changed.

- **M5 Td S151 P1:** made the four-machine fixed-disk result a hard receiver
  sequence: retain `default-at` ATA, IBM 5170 WD1003 and DeskPro Compaq/WD;
  the queued XT audit through final audit must close IBM 5160/Xebec's selected
  `320h`--`323h`, DMA3/IRQ5 route without an ATA shim or duplicate media state.
  The [evidence](../etc/evidence/td-s151-four-machine-fixed-disk-receiver.md)
  records the partition. No runtime, ABI, build or artifact changed.

- **M5 Td S150 P1:** separated the machine-neutral VM resolver kernel from
  profile migration and ordered the queue as Core time boundary, resolver,
  IBM 5170, DeskPro, `default-at`, then YAML. The [evidence](../etc/evidence/td-s150-profile-connection-order.md)
  records the one-way ownership and deleted combined candidate. No runtime,
  ABI, build or artifact changed.

- **M5 Td S149 P1:** withdrew the unimplemented profile-first T473 admission,
  inserted the machine-neutral Core unified guest-time-axis candidate before
  PC/AT 5170 root normalization, and corrected the governance gate to retain
  completed Td identifiers from Status as well as commit history. The indexed
  [evidence](../etc/evidence/td-s149-core-time-before-profile-governance.md)
  records the predecessor/consumer boundary. No runtime, ABI, build or artifact
  changed.

- **M5 Td S148:** re-audits all 145 frozen PIC, DMA, PIT, RTC, KBC,
  8272A FDC, VADP EGA/CGA and ATA/HDC rows under the owner-approved hierarchy:
  direct manual/model facts and real sole-owner external inputs are L3; only
  internal estimates are L2; causal-only relations are L1; absent Core logic
  is L0; source-backed physical timing is L4 and remains out of scope.  The [row-complete evidence](../etc/evidence/td-s148-eight-controller-l-level-reclassification-audit.md)
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
