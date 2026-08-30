# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation `T503 S8`; S7 is accepted after direct IBM/ATA-to-media/DMA/PIC/firmware reconciliation and a 312/312 complete unit replay. |
| Admission And Approval | Owner approved controller-by-controller audit/repair and explicitly requires every S to repair its demonstrable downstream effects without a tail. S8 consumes the selected RTC/CMOS -> PIC/NMI/firmware route; normal commits and non-force pushes are permanently approved. |
| Objective | Directly read the selected RTC/CMOS sources, trace periodic/alarm/update/CMOS/NMI/IRQ8 observation and firmware consumption, cross-check local external emulator logic, then repair the complete RTC-route gap batch. |
| Non-goals | No host-clock injection, profile/firmware/VM workaround, polling loop, second RTC/CMOS state or parallel scheduler. A genuinely independent controller-internal fault transfers only after its causal boundary is proved. |
| Reference Baseline | `85726225` (`M5 T503 S7 P1 reconcile HDC firmware routes`); S1/S2 remain preliminary route inventory only. |
| Candidate Proposal | [M5 controller signal-chain convergence](../proposals/m5-controller-signal-chain-convergence.md), controller-specific RTC/CMOS route S. |
| Files And ABI Surface | Expected `src/core/machine/rtc.[ch]`, immutable RTC/CMOS/PIC/NMI board bindings, owner-local `test/core/` paths and evidence/state/history. Public interface changes require an opaque copied operation; no controller pointer, mutable CMOS state or mutable layout crosses owners. |
| Applicable Rules | `docs/README.md` Task Reading Set; `rules/EXECUTION.md` coverage-bearing, packet, P, review and test rules; `rules/DOCUMENT.md`; source policy; `rules/ARCHITECTURE.md` sole-owner/bounded-interface invariants; `rules/CODING.md` simplicity/test-boundary rules. |
| Verification | Visually inspect and cite applicable Motorola/IBM pages; contrast actual 86Box/MAME/PCjs/Bochs/QEMU RTC logic where available; trace periodic, alarm and update transitions, register-C IRQ acknowledgement, port-70 NMI mask, reset, CMOS reads/writes and firmware-visible forms; run focused owner-local and complete repository-only unit tests. |
| Expected Markers | `T503-S8-RTC-FIRMWARE-ROUTE`; every selected RTC/CMOS observable has one Core state owner, one stated PIC/NMI/firmware consumer route and a source-qualified timing disposition. |
| Asset Needs | Existing primary-source ledgers and read-only external emulator source only if a board edge remains ambiguous; no external bytes enter the repository. |
| Reporting Requirements | Record every selected RTC/CMOS form's manual fact, NXVM route, external comparison, disposition, retained owner and code-size result; report focused/full-unit proof and any earliest-unit transfer. |
| Stop Conditions | Stop for owner direction, authority contradiction, unavailable source needed to classify an RTC/CMOS connection, or a gap independent of the RTC route; do not improvise a workaround. |
| Exit Criteria | Every selected periodic/alarm/update/CMOS/NMI/IRQ8/reset form has primary-source evidence, NXVM trace, external comparison or stated lack thereof, and focused/full-unit proof; every demonstrable downstream effect is repaired in S8 or explicitly shown independent and transferred. |
| Original Owner Request | Each S owns one controller audit and repair, but must also repair affected controllers/devices in the same S; no known causal tail remains. |
| Similar-Issue Sweep | MC146818A register A/B/C/D forms, periodic/alarm/update IRQ flags, port `70h` NMI mask and `71h` CMOS data path, IRQ8 and PIC acknowledgement, reset, invalid CMOS/RAM access, no-host-clock startup and deadline/HLT behavior across 5160, 5170, Model-40 and default-at. |

## Current Technical Baseline

- **Current developer artifact:** CMake target `vm-0-5-0501` emitted
  `nxvm_0_5_0501.exe` in a stripped Release build, SHA-256
  `502D12BE1E30EDBCBE609F424F778A97F665210CA2B3F8B50C88636F8377B2CC`.
  It retains the runtime debugger and contains no compiler debug information.
  Debug uses the repository-only unit route. T471 preserves Core-owned progression:
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
| T502 | Withdrawn before execution by owner direction; no implementation, evidence, test, or artifact change was produced. |
| T501 | Closed: all frozen four-profile media rows have their recorded terminal or external boundary; the Model-40 80386/1.2-MB row reaches `A:\>` through its sole Core/firmware path. [History](../history/M5-T501-four-profile-post-scheduler-media-closure.md). |
| T500 | Closed: existing tests now have one `test/` tree, one unit/integration route, narrow fixture ownership, one product input path, and recovered safe integration parallelism; unit 312/312 and integration 20/20 pass. [Closure](../history/M5-T500-unit-and-integration-test-convergence.md). |
| T499 | Closed: one Core deadline seam owns the current scheduler clients and the selected Model-40 D4 controller; the configured suite passes 302/302 and stripped Release 0499 is recorded. [Closure](../history/M5-T499-core-event-deadline-scheduler-convergence.md). |
| T498 | Closed: the supported-media matrix has semantic terminals or an explicit external-input boundary for every non-Model-40 row, and its Model-40 predecessor transfers to the sole Core scheduler receiver without accepting a shortcut. [Closure](../history/M5-T498-four-profile-supported-media-matrix.md). |
| T497 | Closed: original Model-339 factory 1.2-MB/compatible-360-KB media behavior now has one descriptor/session/FDD path, and the 360-KB DOS installer replay closes its one missing monochrome-aperture mapping; a 286-compatible 1.2-MB external replay transfers to T498. [Closure](../history/M5-T497-ibm-5170-native-floppy-contract.md). |
| T496 | Closed: one Core FDC result/IRQ lifecycle now releases IRQ6 at normal-result acknowledgement, and the selected IBM 5160 DOS terminal plus focused FDC regressions pass without a BIOS/VM/media workaround. [Closure](../history/M5-T496-xt-keyboard-device.md). |
| T495 | Closed: the selected IBM 5160-268 is functionally ready with source-backed L3 relations and explicit L2 limits; 13/13 focused, 300/300 fresh current and specialized gates pass, without a physical/wall-clock overclaim. [Decision](../etc/evidence/t495-s2-xt-final-model-decision.md). |

## Recent Governance

- **M5 T503 S7 P1:** direct IBM/ATA/code/external reconciliation removes the
  inert Xebec `323h` mask cache: one existing HDC owner now gates its existing
  DMA3 request and IRQ5 completion publication, and result consumption clears
  the same IRQ source. The four selected personalities retain separate PIO or
  DMA semantics; focused 7/7 and complete repository-only unit 312/312 pass.

- **M5 T503 S7 P2:** coordinator review accepts `85726225`: direct primary
  and external reconciliation repairs the only HDC-route defect in its sole
  owner, with no ATA alias, second controller/media state or extra scheduler.
  S8 is admitted for the complete RTC/CMOS-to-PIC/NMI/firmware route.

- **M5 T503 S6 P2:** coordinator review accepts `8aac1fb0`: direct
  NEC/IBM/code/external reconciliation adds the sole frozen READY-board input,
  retains one controller-owned reset/DRQ/DMA2/IRQ6 route and corrects the stale
  ready-sampling records; focused 8/8 and complete repository-only unit
  312/312 pass. S7 is admitted for the complete selected HDC route.

- **M5 T503 S5 P2:** coordinator review accepts `9e3b33ba`: direct
  8237A/IBM/code/external reconciliation removes the duplicate raw-primary
  cascade state, retaining one mask-aware derived cascade path; focused 15/15
  and complete repository-only unit 312/312 pass. S6 is admitted for the
  complete 8272A-to-DMA2/PIC/firmware route under the same no-tail rule.

- **M5 T503 S4 P2:** coordinator review accepts `f04a6a4d`: direct
  8253/8254/IBM/code/external reconciliation removes the false OUT-consumer to
  GATE mutation and adds the missing PIT1-to-DMA0 transition proof; focused
  8/8 and complete repository-only unit 312/312 pass. S5 is admitted for the
  complete 8237A-to-provider route under the same no-tail rule.

- **M5 T503 S3 P3:** coordinator review accepts `4dc68983`: the direct Intel
  8259A/IBM/code/external reconciliation retains one PIC-to-CPU path; the
  downstream test-contract repair preserves real DMA ownership of `80h`; the
  isolated repository-only unit gate passes 312/312. S4 is admitted for the
  complete PIT-to-consumer route under the same no-tail rule.

- **M5 Td S155 P1:** closed T498 on its finite matrix/receiver evidence and
  admitted T499's complete Core scheduler-client migration. The Queue retains
  proposal traceability without treating the active task as a candidate;
  documentation governance passes. No runtime behavior, ABI, source or
  artifact changed.

- **M5 Td S154 P1:** admitted the finite four-profile supported DOS-media
  matrix ahead of extender/Windows work, retaining T497 as a closed proposal
  link. It freezes allowed profile/CPU/media rows, semantic terminals and
  external-input boundaries; runtime, source, build and artifact behavior are
  unchanged.

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
