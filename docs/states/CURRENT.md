# Project Status

## Current Work

## M5 T496 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved full XT repair in current T496 on 2026-08-27. S8 is the next complete first-failure batch after S7's selected replay reached a normal 8272A completion then the next BIOS Read Data returned Carry before a third completion. The owner additionally approved immediate Default PC/AT floppy-format repair and real BYOB media verification on 2026-08-27. |
| Objective | Make ordinary 8272A command completion release its Core-owned IRQ source when the guest consumes the first result byte, so each later command can create a new IRQ edge; prove the selected XT 360K replay reaches a DOS terminal without a BIOS, VM, or media special case. |
| Non-goals | No firmware/guest-media import, BIOS compatibility outcome shortcut, VM-owned FDC state, CPU clock fabrication, physical-time claim, profile-side controller, parallel scheduler, image-size geometry guessing, new FDC port, or unrelated FDC/HDC behavior. |
| Reference Baseline | `339e80db` (`M5 T496 S6 P2`) and [XT boot convergence ledger](../etc/evidence/t496-s6-xt-boot-convergence-ledger.md), B3+B8. |
| Candidate Proposal | [XT keyboard-device proposal](../proposals/ibm-5160-xt-keyboard-device.md), Bootability Completion Boundary. |
| Files And ABI Surface | Existing Core 8272A state and its PIC source, plus owner-local FDC/XT tests and evidence only. No public Core/VM ABI extension. |
| Applicable Rules | NEC uPD765/IBM 5160 source hierarchy; Core sole mutable FDC/PIC-source owner; VM-to-Core construction-only flow; one command/result/IRQ lifecycle; result simplicity and source policy. |
| Verification | Focused FDC coverage proves completion asserts IRQ, first normal result-byte consumption deasserts only the FDC source while preserving all result bytes, and the next completion asserts again. Release replay proves the selected XT reaches a DOS terminal; Default PC/AT terminal matrix remains valid. BYOB records only terminal semantics. |
| Progress Evidence | S7's claimed selected XT installer terminal was not reproducible in the current Release replay and is superseded. The replay showed two successful FDC transfers, a correct IO.SYS/MSDOS.SYS comparison, then Carry from the next INT 13h Read Data before its expected completion. S8 releases the Core-owned FDC IRQ6 source on first ordinary result-byte consumption; the selected XT replay now reaches the installer terminal after 161 successful FDC completions. NEC establishes the completion INT/result contract; local PCjs is corroborating Other L3 for first-result-byte release. See [S8 evidence](../etc/evidence/t496-s8-fdc-result-irq-release.md). |
| Expected Markers | `FDC-RESULT-IRQ-RELEASE:OK`, `XT-DOS-360K:PASS`, plus the existing Default PC/AT media terminals. |
| Asset Needs | Owner-authorized firmware and DOS media are external runtime inputs only; records contain only semantic results. |
| Reporting Requirements | Record source relation, one owner/path, focused proof, code-size accounting, replay semantic result and every remaining batch transfer; do not record protected material details. |
| Stop Conditions | Stop before code change if the FDC source lifecycle is not reproducible or if repair requires BIOS/VM/media special behavior. If the repaired replay stops at a later terminal, transfer that next complete failure batch without conflating it with this one. |
| Exit Criteria | One Core FDC result path releases its sole PIC source at the defined normal-result acknowledgment boundary, focused regressions prove repeated completions, and selected XT replay reaches a reviewed DOS terminal. No BIOS/VM release path exists. |
| Original Owner Request | Fully repair XT so it actually starts DOS; do not accept a 60-second timeout as normal performance. |
| Similar-Issue Sweep | Search every FDC normal-result producer and result-byte consumer plus reset/seek Sense Interrupt release paths. Classify each production hit as fixed, distinct, or deferred; no normal completion retains an undocumented IRQ source assertion. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0494`; the stripped Release
  `nxvm_0_5_0494.exe` has SHA-256
  `C7BDFD1CE67917373DCD762E093D8A5FF86798DC4E59DEF162B5682EB6B0686A`.
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
| T496 S6 | Complete: frozen B1--B8 DOS-startup convergence ledger and selected the source-backed B3+B8 PIT1/DMA0/scheduler mechanism batch; later checkpoints cannot hide another batch. [Ledger](../etc/evidence/t496-s6-xt-boot-convergence-ledger.md). |
| T495 | Closed: the selected IBM 5160-268 is functionally ready with source-backed L3 relations and explicit L2 limits; 13/13 focused, 300/300 fresh current and specialized gates pass, without a physical/wall-clock overclaim. [Decision](../etc/evidence/t495-s2-xt-final-model-decision.md). |
| T494 | Closed: the complete IBM 5160 Xebec source/List-1/List-2/sole-owner chain corrects Read block-count progression without a second controller/media path; full current gate passes 300/300 and stripped Release 0494 is recorded. [Closure audit](../etc/evidence/t494-s5-xebec-closure-audit.md). |
| T493 | Closed: complete IBM CGA source/List-1/List-2/one-owner repair reconciles all 33 rows; the generic latch and binary high-resolution palette repair leave no in-scope tail. [Closure audit](../etc/evidence/t493-s5-cga-closure-audit.md). |
| T492 | Closed: complete IBM 5160 8272A/logical-media source/List-1/List-2/sole-owner chain removes fabricated reset state and global timing literals; serial gate passes 300/300 and stripped Release 0492 is recorded. [Closure audit](../etc/evidence/t492-s5-8272a-closure-audit.md). |
| T491 | Closed: the independent IBM 5160 8255 PPI/key/NMI unit maps all 19 rows to one Core owner path, closes PB0/PB1 through the existing PIT2/speaker consumer, and records stripped Release 0491. [Closure audit](../etc/evidence/t491-s5-8255-closure-audit.md). |
| T490 | Closed: the independent IBM 5160 8253 unit has verified source/List-1/List-2/sole-owner implementation, a corrected full gate and stripped Release 0490 artifact. [Closure audit](../etc/evidence/t490-s6-8253-closure-audit.md). |
| T489 | Closed: complete IBM 5160 8237A source/List-1/List-2/one-owner batch closes the single page-port gap without new state or parallel path; physical-axis conversion remains transferred. [Closure audit](../etc/evidence/t489-s5-8237a-closure-audit.md). |
| T488 | Closed: complete IBM 5160 8259A source/List-1/List-2/one-owner audit retains one Core PIC path and an empty implementation batch. [Closure audit](../etc/evidence/t488-s5-8259a-closure-audit.md). |

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
