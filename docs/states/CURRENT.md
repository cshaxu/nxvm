# Project Status

## Current Work

## M5 T461 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved single-person dual-role execution of T461 on 2026-08-24. Coordinator accepts S2 implementation `a1ea0fa1` after actual diff, focused 4/4 and complete 294/294 current-gate review, and admits S3 only. |
| Objective | Prove the existing sole PIT channel-0 OUT to PIC IRQ0 binding, deterministic scheduler visibility and reset/finalize lifecycle; close T461 without elevating board-order or physical-clock facts beyond their documented L2 boundary. |
| Non-goals | No PIT chip state-machine redesign, host-clock pacing, oscillator/electrical claim, speaker/PPI/audio implementation, second scheduler, profile wiring change, public ABI addition, or external-code import. |
| Reference Baseline | `a1ea0fa1`; accepted T456 PIC and T460 DMA contracts; accepted T461 S1 Lists 1--2 and S2 phase evidence. |
| Candidate Proposal | [Core PIT 8254 phase contract](../proposals/m5-core-pit-8254-phase-contract.md). |
| Files And ABI Surface | `machine.c`/scheduler composition and existing PIT/PIC focused smokes only; existing Core-local API, no public ABI. |
| Applicable Rules | README Task Reading Set; EXECUTION S/P lifecycle; DOCUMENT governance; architecture single-owner rule; coding no-wrapper/owner-local repair rule; source policy; specification-driven L3 timing design. |
| Verification | Sweep every PIT output provider, channel-0 binding and scheduler consumer; run the focused PIT/IRQ0/divider corpus, full current-gate, documentation gate and diff hygiene. |
| Expected Markers | `M5:T461:S3:PIT-INTEGRATION-CLOSURE:OK`; P15 has one owner, named focused proof and every retained L2 boundary is explicit. |
| Asset Needs | T461 S1 admitted Intel manual evidence; read-only external observations only as corroboration, never import. |
| Reporting Requirements | Record the output-to-PIC route, ordering owner, reset/finalize release path, code-size delta and each retained L2 boundary. |
| Stop Conditions | Stop for a physical time derivation, an unselected topology, a required second mutable owner or any material chip-contract defect outside the accepted S2 evidence. |
| Exit Criteria | P15's chip OUT and selected Core binding are proven through one route; scheduler order/reset are either proven or retained as L2 with receiver; no duplicate path/state/API is introduced; full gate and documentation gate pass; closure evidence, history and current artifact are recorded and accepted. |
| Original Owner Request | Execute the current PIT task in single-person dual-role mode, using global minimalist design and no accretion. |
| Similar-Issue Sweep | Sweep every PIT output provider, channel-0 binding, PIC timer consumer, scheduler caller and reset/finalize release path; classify every hit at its one owner. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0460`; `nxvm_0_5_0460.exe` / `build/output/nxvm_0_5_0460.exe`, SHA-256 `29FB7AC3D715B45D60A82F4D32F3B4D17C8B4A8601C60FBE482DBB332CE0AF62`. It is built only from the stripped Release current-artifact route; Debug remains the current-gate route. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T461 S1 | Accepted at `1c9428b1`: manual-first 8254 Lists 1--2 dispose every selected row as Manual-L3 or explicit L2; `pit.c` remains the sole owner and S2 is admitted for the control/latch contract. [Evidence](../etc/evidence/t461-s1-pit-8254-function-timing-ledger.md). |
| T461 S2 | Accepted at `a1ea0fa1`: one PIT owner now covers P1--P14 CR-to-CE, latches, modes, GATE and CE phases; focused 4/4 and full current-gate 294/294 pass. [Evidence](../etc/evidence/t461-s2-pit-chip-phase-contract.md). |
| T460 | Closed: all 16 DMA rows are disposed through the sole `dma.c` owner; normal/compressed and M2M service phases have focused proof, the five-clock conversion remains explicit L2, serial current-gate is 294/294, and the stripped 0460 artifact is recorded. [History](../history/M5-T460-core-dma-8237a-phase-contract.md). |

| T459 | Closed after S2 correction: `standard`/`turbo` remain stopped-session Console selections, but neither VM speed branch manufactures guest ticks. `Sleep(1)` is explicitly L2 HLT host-load backoff; true Standard pacing and Turbo fast-forward remain transferred to Core deadlines plus profile timebases. The full gate passes 294/294; current artifact `0.5.0459` is stripped Release-only. [History](../history/M5-T459-vm-session-speed-policy.md). |
| T458 | Closed: the shared runner retains its 256-instruction control quantum but captures/publishes normal frames no more than once per 16 host milliseconds; forced mode and lifecycle frames remain immediate. The current artifact is stripped Release-only, and the full gate passes 293/293. [History](../history/M5-T458-vm-runner-presentation-cadence.md). |
| T457 | Closed: F9 remains a single host run-handle stop report and no longer enters the guest key route; the corrected host-cancellation assertion and two-epoch regression preserve it. Its artifact is stripped Release-only, retains the production debugger/trace commands, excludes automatic Core trace events, and 293/293 current-gate tests pass. [History](../history/M5-T457-session-input-restart-recovery.md). |
| T456 | Closed at L3: all 18 retained 8259A rows have focused proof or an explicit boundary; one CPU path, programmed cascade, corrected specific-EOI, command/poll lifecycle and default-IR7 acknowledgement remain PIC-owned. Full current-gate is 293/293. PIC-T3 L4 electrical timing is deliberately excluded. [History](../history/M5-T456-core-pic-8259a-phase-contract.md). |
| T455 | Closed: the sole 8086 decoder-ledger guard now checks current decoder and timing owners instead of the retired monolithic layout; all 1,053 records and 292 current-gate tests pass. [History](../history/M5-T455-8086-decoder-ledger-guard-reconciliation.md). |
| T454 | Closed: the three fixed-write Console catalog smokes have one CTest-owned build workspace each; serial and repeated parallel replays pass with no source-tree residue. The independent 8086 decoder-ledger guard failure is recorded as CPU debt. [History](../history/M5-T454-parallel-console-profile-smoke-isolation.md). |
| T453 | Closed: fresh default tree uses one WinLibs toolchain family; the one FDC test include is corrected; 44 obsolete build trees are removed; current build operations are reduced to preset-backed commands. [History](../history/M5-T453-toolchain-build-tree-hygiene.md). |

## Recent Governance

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
