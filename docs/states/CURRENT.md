# Project Status

## Current Work

## M5 T466 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner goal dated 2026-08-25 directs completion of the remaining controller tasks; accepted T466 S7 permits the next finite VADP-F2 graphics-mode batch. |
| Objective | Reconcile the complete VADP-F2 IBM mode-layout batch against current code and prove the earliest single-owner receiver for the observed D/E/10 snapshot and generated-firmware port-programming gaps. |
| Non-goals | No speculative geometry heuristic, F-mode odd/even implementation, alpha character generation, BIOS mode parser, VADP clock conversion, monitor model, board contention, profile policy or mutable mode setter. |
| Reference Baseline | `9cf7577d`; IBM EGA pp. 5--10, VADP-F2 ledger/audit, current planar provider/snapshot path, mode-10 and EGA planar tests. |
| Candidate Proposal | [Core VADP phase contract](../proposals/m5-core-vadp-phase-contract.md). |
| Files And ABI Surface | VADP F2 checklist/audit/history/status and focused test evidence only. The next implementation S owns `vadp.c`, generated firmware and exact D/E/10 tests; no public configuration API or additional state owner. |
| Applicable Rules | IBM manual is primary. Core VADP exclusively owns register/plane state and copied snapshot construction. CRTC programmed geometry, not a guessed BIOS number or a host renderer, selects the frame arrangement. Preserve one VRAM/provider route and one copied-frame boundary. |
| Verification | Visually review IBM mode and memory-layout pages; cross-check limited layout facts against labelled external EGA/VGA models; replay direct and VM mode-10 evidence before and after the audit; run diff and documentation gates. |
| Expected Markers | Every F2 mode class has a Manual L3 or explicit L2 disposition; current offset-only D/10 classification and the failing generated-firmware-to-VADP observation are recorded as one in-scope receiver, not concealed by a new renderer or BDA coupling. |
| Asset Needs | No source, firmware, display capture or guest-media import; temporary manual renders stay untracked. |
| Reporting Requirements | Record the exact Manual L3 mode facts, limited external corroboration, the mode-10 replay result, one earliest implementation receiver, and zero production code lines in this reconciliation S. |
| Stop Conditions | Stop if the manual does not distinguish a layout, if the receiver would require a second renderer/VRAM route, or if an external model conflicts with the IBM layout rather than clarifying an explicit L2 boundary. |
| Exit Criteria | The finite F2 layout batch is fully disposed, including a reproducible current mode-10 failure and one next implementation scope that will repair the production route without a BDA/host shortcut. |
| Original Owner Request | Complete remaining controllers source-first with minimalist single-owner implementation and explicit L2 rather than guessed timing. |
| Similar-Issue Sweep | Inspect all display-kind switches, planar snapshot dimensions/stride/start address, CRTC register use, direct EGA mode setup, VM copied-frame consumers, CECG snapshots and tests for assumptions that offset 28h means only mode 10. |

| Task | Compact progress |
| --- | --- |
| T466 S7 | Accepted: IBM's 64/128/256-KiB capacity and aperture facts are Manual L3; no source establishes the exact extended-bank CPU mapping, so current unconditional backing and unselected card capacity are explicit L2 rather than a false 86Box-derived L3. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0465`; `nxvm_0_5_0465.exe` / `build/output/nxvm_0_5_0465.exe`, SHA-256 `410981B728E7FEE4BC199C7593AF0DE2A692546A22AB2D2A5B62988600C243C9`. It is built only from the stripped Release current-artifact route; Debug remains the current-gate route. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T466 S4 | Accepted: IBM EGA external routing, Sequencer Reset and CRTC 00h--18h use the sole VADP/memory route; clock, IRQ and physical inputs remain L2. |
| T465 | Closed: selected Intel 8272A source reconciliation, command/reset repair and per-drive parallel Seek close the logical controller contract; physical media and board-time conversion remain explicit L2. Release 0465 is stripped and the 294/294 serial gate is recorded in [history](../history/M5-T465-core-intel-8272a-logical-media-phase-contract.md). |
| T464 | Closed: all sixteen KBC rows are Manual L3 or explicit L2/L4. One Core owner now covers selected command/input/test state; 294/294 serial current-gate tests pass and stripped Release 0464 is recorded in history. |
| T463 | Closed: the 16-row MC146818A/IBM AT ledger has one Core phase owner, a copied L3/L2 board timing plan, explicit L2/L4 boundaries and focused proof. The full current gate ran 294/294; stripped Release 0463 is recorded in [history](../history/M5-T463-core-rtc-cmos-phase-contract.md). |
| T462 | Closed: one immutable copied plan carries only qualified Model-339 PIT/DMA selections; PIC remains explicit L2. The seven corrected PIT-transition regressions, 294/294 serial current gate, 77/77 specialized gates, documentation governance and stripped 0462 artifact complete the controller-board contract. [History](../history/M5-T462-core-controller-board-timing-contract.md). |
| T461 | Closed: one Core PIT owner covers manual P1--P14 and one `OUT0 -> IRQ0 -> PIC refresh` route closes P15; P16--P18 remain explicit L2. Full current-gate is 294/294 and its stripped artifact is retained in history. [History](../history/M5-T461-core-pit-8254-phase-contract.md). |
| T460 | Closed: all 16 DMA rows are disposed through the sole `dma.c` owner; normal/compressed and M2M service phases have focused proof, the five-clock conversion remains explicit L2, serial current-gate is 294/294, and the stripped 0460 artifact is recorded. [History](../history/M5-T460-core-dma-8237a-phase-contract.md). |

| T459 | Closed after S2 correction: `standard`/`turbo` remain stopped-session Console selections, but neither VM speed branch manufactures guest ticks. `Sleep(1)` is explicitly L2 HLT host-load backoff; true Standard pacing and Turbo fast-forward remain transferred to Core deadlines plus profile timebases. The full gate passes 294/294; current artifact `0.5.0459` is stripped Release-only. [History](../history/M5-T459-vm-session-speed-policy.md). |
| T458 | Closed: the shared runner retains its 256-instruction control quantum but captures/publishes normal frames no more than once per 16 host milliseconds; forced mode and lifecycle frames remain immediate. The current artifact is stripped Release-only, and the full gate passes 293/293. [History](../history/M5-T458-vm-runner-presentation-cadence.md). |
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
