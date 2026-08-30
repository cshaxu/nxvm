# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T512; S1--S4 accepted, S5 is active. |
| Admission And Approval | Owner approved the five-CPU complete audit after Td S157 removed the already-completed D4 candidate. |
| Objective | Reconcile every decoder-admitted 8086 and 8088 real-mode form, legal prefix, FLAGS/frame effect and source-backed timing row at its existing sole Core owner. |
| Non-goals | Do not derive 8088 bus behavior from 8086, alter 80186-or-later semantics, add profile-side CPU behavior, assert a value for undefined state, import third-party source, or change `build/output` YAML. |
| Reference Baseline | Current Core decode/execution, existing per-CPU ledgers, T499 CPU/profile audit history, and stripped Release 0511 baseline. |
| Candidate Proposal | [Five-CPU complete instruction re-audit and repair](../proposals/m5-five-cpu-complete-instruction-reaudit.md). |
| Files And ABI Surface | Decoder, execution, FLAGS/frame, retirement and timing-selector owners reached by the finite 8086/8088 List-2 rows, plus their owner-local test modules. No public ABI, VM profile, 80186-or-later path or artifact-template change. |
| Applicable Rules | Task Reading Set; execution, architecture, coding, source-policy and documentation authorities. Intel originals are normative; external emulator code is read-only corroboration only. |
| Verification | Every S closes with the complete repository-only unit suite and documentation governance; T closure also runs external-ROM/disk integration and builds stripped Release 0512. |
| Expected Markers | Every 8086/8088 List-2 row has a Manual-L3, explicit lower-tier, unsupported or single-owner repair disposition; shared real-mode behavior stays one Core path and 8088 bus/timing remains source-distinct. |
| Asset Needs | Read-only owner-managed Intel 8086 and 8088 manuals under `assets/manuals`; no manual, ROM, guest media or third-party source import. |
| Reporting Requirements | Record each source locator, exact row/owner sweep, any removed duplicate path, source/test code-size delta and full unit result. |
| Stop Conditions | Stop if a source-distinct 8088 timing/bus behavior lacks a represented Core owner, or a candidate repair crosses an 80186-or-later boundary; revise the S brief rather than infer or broaden it. |
| Exit Criteria | All 8086/8088 List-2 rows are reconciled against source and current owners; each proven repair is sole-owner and swept across equivalent forms; full unit and documentation governance pass. |
| Original Owner Request | Audit and correctly repair all supported five-CPU instruction, architectural-state and timing behavior using manual-first evidence and minimal single-owner design. |
| Similar-Issue Sweep | Sweep every 8086/8088 decoder-admitted form and its prefix, state/frame, delivery, retirement and timing selector owner; trace shared owners once and distinguish the 8088 bus/timing rule from the 8086 rule. |

## Current Technical Baseline

- **Current developer artifact:** CMake target `vm-0-5-0511` emitted
  `nxvm_0_5_0511.exe` in a stripped Release build, SHA-256
  `3A5F62DF4CF4F790829C6D26AEBA12DBF9D434129D9DC17855DEF9099DBEE1A4`.
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
| T510 | Closed: ATA, WD1003, Compaq/WD and Xebec retain one HDC/media owner with explicit `200/200`, `16000/7840`, `0/0` and `250/0` service dispositions. The 0508 ATA PIO consumer regression is repaired; actual HDD Windows Setup reaches Welcome. Unit 313/313, integration 20/20, governance and stripped Release 0508 pass. [History](../history/M5-T510-hdc-personality-service-deadline-closure.md). |
| T509 | Closed: all 33 session-backed test runners and the production runner have a deadline-consumer, explicit-HLT, or bounded non-continuation disposition. Seven long-running runners now reuse the existing VM waiting boundary; unit 313/313, integration 20/20, governance and stripped Release 0508 pass. [History](../history/M5-T509-session-deadline-consumer-convergence.md). |
| T508 | Reclosed: Core accepts one provenance-neutral copied timing plan; default ATA has an Other-L2 controller quantum and Model-40 an Other-L2 macro pacing axis. The four-profile matrix reaches only Core deadlines, never the L1 escape. Unit 313/313, integration 20/20, governance and stripped Release 0508 pass. [History](../history/M5-T508-core-timing-input-qualification-closure.md). |
| T507 | Corrective S14 closed: copied DMA ratios publish deadlines; FDC, HDC and D4 retain one Core-owned deadline/owner route without a VM clock or controller mirror. Unit 312/312, integration 20/20, governance and stripped Release 0507 pass. [History](../history/M5-T507-core-scheduler-standard-turbo-performance-closure.md). |
| T506 | Closed: selected 8237A D1--D10 relations are reconciled; Model-339 can advance to the next sourced DMA phase through the existing Core scheduler while unqualified contracts remain isolated. Unit 312/312, integration 20/20 and stripped Release 0506 pass. [History](../history/M5-T506-dma-service-arbitration-deadline-closure.md). |
| T505 | Closed: Intel-backed cascade publication and withdrawal are immediate PIC-owner work, not a scheduled L1 tick. One construction-fixed PIC pair reuses the existing refresh calculation after source and command transitions; full unit 312/312 and integration 20/20 pass. [History](../history/M5-T505-pic-cascade-observable-progress-closure.md). |
| T504 | Closed: one Core-owned bounded Turbo fallback advances only copied L1 no-deadline waits; Standard receives no compatibility progression or host-to-guest ticks. Same-input external DOS replays reach `A:\>` in both modes; formal Release output ships selectable strict-grammar templates; the four controller-specific timing receivers are queued. [History](../history/M5-T504-turbo-l1-compatibility-escape.md). |
| T502 | Withdrawn before execution by owner direction; no implementation, evidence, test, or artifact change was produced. |

## Recent Governance

- **M5 Td S157 P1:** removes the obsolete Model-40 D4 refresh-hold candidate:
  T419 already closes PIT1-to-D4-to-DMA ordering and T507 S14 publishes its
  one next-Core-step L2 deadline. Physical D4 DRAM/ISA timing remains separate
  debt; no duplicate task is admitted. See
  [evidence](../etc/evidence/td-s157-d4-candidate-reconciliation.md).

- **M5 Td S156 P1:** removes all Release post-build YAML copying. `build/output`
  configuration is user-managed and no executable build may overwrite it; no
  YAML restoration or mutation is attempted.

- **M5 T510 S3 P5:** coordinator actual-diff review accepts `5182e2c6` and
  closes T510. The 0508 regression was a sole firmware multi-sector PIO
  consumer defect, not an HDC timing defect: every sector now observes the
  same Core-owned BSY/ERR/DRQ transition. The two-sector read/write proof,
  actual Setup-entry checkpoint, unit 313/313, integration 20/20,
  documentation governance and stripped Release 0508 all pass.

- **M5 T510 S2 P2:** coordinator actual-diff review accepts `53698062`.
  The generic two-transition HDC input eliminates the misleading shared scalar;
  Xebec DCB completion uses the existing sole Core deadline route, while the
  Model-40 zero boundary remains explicit. Focused 8/8, unit 313/313 and
  documentation governance pass. Task-level integration and closure remain
  required before T510 can close.

- **M5 T510 S1 P2:** coordinator actual-diff review accepts `09a83060`.
  The finite ATA/WD1003/Compaq/Xebec ledger uses original sources for each
  host-visible contract, confines 86Box timing to explicitly labelled Other-L2
  observations, and identifies one Core-HDC batch with no second owner,
  scheduler or profile-side state. Documentation governance and unit 313/313
  pass.

- **M5 T509 S1 P2:** coordinator actual-diff review accepts `21dc1c3a`.
  The finite 33-runner ledger retains one Core deadline owner and one VM
  consumer boundary; seven long-running test runners now consume it, while
  explicit-HLT and bounded non-continuation tests remain distinct. Unit
  313/313, integration 20/20, documentation governance and stripped Release
  0508 pass.

- **M5 T508 S4 P2:** coordinator actual-diff review accepts `f9373778`.
  The provenance-neutral HDC input, sole plan-copy construction path and
  four-profile pending-DMA deadline matrix are accepted; unit 313/313,
  integration 20/20, governance and stripped Release 0508 pass.

- **M5 T507 S14 P2:** corrective closure accepts the finite DMA/FDC/HDC/D4
  sweep after independent diff review: unit 312/312, integration 20/20,
  documentation governance and stripped Release 0507 pass.

- **M5 T507 S14:** owner-approved corrective reopening after review found the
  literal pending-DMA L1 predicate also supersedes a qualified DMA deadline.
  This S consumes the complete active DMA/HDC/FDC/D4 blocker sweep before any
  later queue candidate may be admitted.

- **M5 T507 S13 P2:** coordinator actual-diff review accepts `a3574918` and
  closes T507.  The complete C1--C13/V1--V4 matrix, the Core-only L1/deadline
  composition, 312/312 unit, 20/20 integration and stripped 0507 artifact are
  recorded in the T507 history; HDC L1 service timing transfers to the next
  independent candidate.

- **M5 T507 S13 P1:** one Core progression path is verified across Standard
  and Turbo: focused scheduler/speed 4/4, unit 312/312 and external-asset
  integration 20/20 pass.  Release 0507 is `-O3`/`-DNDEBUG`, stripped of
  compiler debug sections and retains the runtime debugger.

- **M5 T507 S12 P2:** coordinator actual-diff review accepts `41254f67`.
  Core now gives explicit L1 owners the existing bounded progression before an
  unrelated deadline, while Standard/Turbo share that Core path and differ
  only in host waiting; focused 4/4 and full unit 312/312 pass.

- **M5 T507 S11 P2:** coordinator actual-diff review accepts `bbe134aa`.
  Model-40 D4 refresh-hold remains one board request and one Core transaction
  before DMA, with its no-duration L1 boundary explicit; focused 5/5 and full
  unit 312/312 pass.

- **M5 T507 S10 P2:** coordinator actual-diff review accepts `6f4718c9`.
  ATA, WD1003, Compaq/WD and Xebec retain distinct sole-owner routes and one
  shared explicit L1 service boundary; focused HDC 6/6 and full unit 312/312
  pass with no guessed delay or profile-side progression.

- **M5 T507 S9 P2:** coordinator actual-diff review accepts `e704100f`.
  IBM EGA Attribute display-enable now gates its existing text fallback and
  recognized planar frame paths through one VADP predicate; focused proof and
  full unit 312/312 pass without a VM/renderer state path.

- **M5 T507 S8 P2:** coordinator actual-diff review accepts `c9a7af52`.
  Intel's explicit FM 31-us/MFM 15-us service bounds now select one existing
  FDC byte gate through the command MFM bit; FDC, DMA2, IRQ6, reset and the
  Core deadline receiver remain sole-owner paths. Focused FDC proof, full unit
  312/312 and documentation governance pass.

- **M5 T507 S6 P2:** coordinator actual-diff review accepts `81cad8c2`.
  Motorola RESET semantics are repaired in the sole RTC owner by preserving
  its existing calendar/divider phase; no host time, second calendar, profile
  workaround or scheduler route was added. Focused RTC 9/9, full unit 312/312
  and documentation governance pass.

- **M5 T507 S5 P2:** coordinator actual-diff review accepts `723477c8`.
  Original Intel and IBM sources, current code and available local references
  confirm one PIT state/output/deadline owner and direct consumer routes; no
  redundant timer, speaker, refresh or scheduler path was added. Focused PIT
  9/9, full unit 312/312 and documentation governance pass.

- **M5 T507 S4 P2:** coordinator actual-diff review accepts `8f13d08d`.
  The Intel/IBM and available-local implementation reconciliation confirms one
  Core DMA owner and one scheduler observation path; no duplicate timer,
  request path, public ABI or VM/profile workaround was added. Focused DMA
  5/5, full unit 312/312 and documentation governance pass.

- **M5 T507 S3 P4:** coordinator accepts e48125b0 after the P3
  governance-only mojibake correction. The complete Intel 8259A surface maps
  to the sole PIC owner or its electrical L2 boundary; focused PIC 6/6, full
  unit 312/312 and documentation governance pass.

- **M5 T506 S3 P2:** coordinator actual-diff review accepts `7d197d3f` and
  closes T506. All selected D1--D10 rows map to the sole DMA/Core path or their
  stated lower-tier boundary; unit 312/312, integration 20/20, documentation
  governance and stripped Release 0506 pass. The next candidate is HDC.

- **M5 T506 S2 P2:** coordinator actual-diff review accepts `521085bd`.
  The repair adds one scheduler predicate and one deadline consideration for
  the already qualified Model-339 plan; it leaves the sole DMA state/service,
  public ABI, FDC/Xebec/PIT routes, unqualified profiles, and Model-40 BUSRDY
  contract untouched. Complete unit 312/312 and documentation governance pass.

- **M5 T506 S1 P2:** coordinator actual-diff review accepts `9905ff39`.
  Intel/IBM material and available-local 86Box/Bochs/PCjs corroboration bound
  D1--D11, retaining exactly D10 for a whole-owner deadline evaluation; no
  source, timing value, external code, VM/profile path, or second DMA owner
  was added. Complete unit 312/312 and documentation governance pass.

- **M5 T505 S2 P2:** coordinator actual-diff review accepts `afc1d540` and
  closes T505. The sole PIC owner refreshes the existing derived cascade state
  at existing source and command mutation points; no timer, VM/profile route,
  second dispatcher, or duplicated state exists. Complete unit 312/312,
  integration 20/20 and documentation governance pass.

- **M5 T505 S1 P2:** coordinator actual-diff review accepts `d0f510fa`.
  Intel 8259A logic and available 86Box/Bochs/PCjs corroboration select one
  PIC-local immediate publication/withdrawal receiver, with no inferred delay,
  VM/profile route, second dispatcher, or host-time path. Complete unit
  312/312 and documentation governance pass.

- **M5 T504 S5 P3:** coordinator actual-diff review accepts `939dfce1` and
  `79e6b2c0`, then closes T504. The stripped Release same-input DOS replays
  reach `A:\>` in Standard and Turbo, and current session templates are
  delivered beside the executable. Core remains the sole time/owner path; PIC,
  DMA, HDC and Model-40 D4 receive their unresolved owner timing relations as
  distinct queued tasks.

- **M5 T503 S7 P1:** direct IBM/ATA/code/external reconciliation removes the
  inert Xebec `323h` mask cache: one existing HDC owner now gates its existing
  DMA3 request and IRQ5 completion publication, and result consumption clears
  the same IRQ source. The four selected personalities retain separate PIO or
  DMA semantics; focused 7/7 and complete repository-only unit 312/312 pass.

- **M5 T503 S7 P2:** coordinator review accepts `85726225`: direct primary
  and external reconciliation repairs the only HDC-route defect in its sole
  owner, with no ATA alias, second controller/media state or extra scheduler.
  S8 is admitted for the complete RTC/CMOS-to-PIC/NMI/firmware route.

- **M5 T503 S8 P1:** direct Motorola/IBM and 86Box/PCjs/Bochs reconciliation
  finds one real RTC-route defect: AIE-only alarms asserted AF/IRQ8 in the
  owner but were omitted from its existing deadline query.  The RTC owner now
  calculates that one alarm deadline through its existing calendar predicate;
  no second calendar, scheduler, NMI route, profile workaround or host clock
  path is added.  The rebuilt RTC-focused cohort passes 4/4 and the complete
  repository-only unit replay passes 312/312.

- **M5 T503 S8 P2:** coordinator actual-diff review accepts `caba86b2`.  Its
  one RTC-local alarm-deadline helper reuses the sole calendar and alarm
  predicate, so AIE no longer leaves HLT with an omitted connected IRQ8 event.
  No other S8 batch member requires a second path or corrective receiver.

- **M5 T503 S9 P1:** direct Intel/IBM and available-local-emulator reconciliation
  finds one XT keyboard-route defect: BAT completion reused its already spent
  ticks to consume the first serial edge.  The sole XT keyboard owner now
  consumes each phase once, retaining the PPI byte/IRQ1 path and all existing
  8042/XT topology separation.  Focused 6/6 and complete repository-only unit
  312/312 proof passed for this implementation delivery.

- **M5 T503 S9 P2:** coordinator actual-diff review accepts `14757a14`.  Its
  phase-consuming loop retains the single XT keyboard/PPI/PIC route and makes
  BAT expiry followed by the sourced first serial-edge delay observable; no
  KBC/XT alias, duplicate state, profile workaround or scheduler was added.

- **M5 T503 S10 P2:** coordinator actual-diff review accepts `8d0692a2` and
  `e9184464`: direct IBM/available-local model reconciliation corrects CGA
  `3D9h` RGBI foreground and VADP-owned EGA display-disable publication,
  including the text fallback. The result retains one VADP port/VRAM/frame
  state and one copied VM presentation consumer; focused 12/12 and complete
  unit 312/312 pass. S11 is the cross-profile closure batch.

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
