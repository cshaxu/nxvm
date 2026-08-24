# Project Status

## Current Work

**Active: M5 T456 S5 - PIC spurious acknowledgement correction.**

## M5 T456 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective |
| Admission And Approval | Owner explicitly directed the repair on 2026-08-24 after the T456 closure claim was found to omit the manual's no-request acknowledge/default-IR7 behavior. This is a narrow defect in the most recently closed numeric task. |
| Objective | Make the sole PIC acknowledgement operation return the master default IR7 vector when it finds no request, without manufacturing an ISR transition or a second CPU delivery path. |
| Non-goals | Do not model electrical spurious-race timing, async request withdrawal, slave-specific race variants, a new CPU route, topology policy, ELCR/APIC, or a physical INTA waveform. |
| Reference Baseline | `fe091cac`; T450 `PIC-F8`; T456 S1 rendered Intel 8259A `231468-003` finding that no request at acknowledge yields default IR7 behavior. |
| Candidate Proposal | [PIC default-IR7 correction](../proposals/m5-pic-default-ir7-correction.md); [retained T456 proposal](../history/M5-T456-core-pic-8259a-phase-contract-proposal.md). |
| Files And ABI Surface | Expected PIC-local `pic.c`, existing PIC command smoke, T456 evidence/history/index and Current. No public VM/profile ABI or new source/artifact. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/DOCUMENT.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, design architecture/coding, T450 PIC-F8 and T456 S1/S2 evidence. PIC owns acknowledgement state; CPU remains the sole ordinary delivery consumer. |
| Verification | Prove initialized-master no-request `get` returns ICW2 base plus IR7 with no IRR/ISR mutation; prove ordinary master and slave acknowledgement remain unchanged; run affected PIC/CPU smokes, configured build, full current-gate and documentation governance. |
| Expected Markers | `get` has one explicit no-selection default-vector result; it does not set ISR7, clear unrelated IRR, alter scan/peek semantics, or bypass the S2 CPU gate. |
| Asset Needs | Existing admitted Intel source, local code and existing test fixture only. |
| Reporting Requirements | Record source-to-behavior mapping, caller/race-boundary sweep, before/after observable state, code delta, gates and retained physical/slave-race boundary. |
| Stop Conditions | Stop if exact behavior needs an asynchronous electrical race model, a second acknowledgement API, public composition change or unsourced slave policy; retain it as a separate boundary. |
| Exit Criteria | A focused no-request acknowledgement regression passes, ordinary PIC paths remain green, all gates pass, and T456 closure is corrected without overclaiming physical timing. |
| Original Owner Request | Implement 8259A manual-level function/timing behavior with minimalist single-owner design; after correction request, repair the omitted default IR7 behavior immediately. |
| Similar-Issue Sweep | Sweep every `core_machine_pic_get_interrupt` caller/test, `scan`/`peek` no-selection handling, vector construction, ISR mutation and all references to spurious/default IR7; classify slave-race and physical timing separately. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0448`; `nxvm_0_5_0448.exe` / `build/output/nxvm_0_5_0448.exe`, SHA-256 `A294EA4AEBBB14C7D44B5F2141F6E457505C67808D25E985ECC37AE823BA7462`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T456 | Closed: all 18 retained 8259A rows have a focused proof or explicit L2/L4 boundary; one CPU delivery path, programmed cascade, corrected specific-EOI and command/poll lifecycle remain PIC-owned. Full current-gate is 293/293. [History](../history/M5-T456-core-pic-8259a-phase-contract.md). |
| T455 | Closed: the sole 8086 decoder-ledger guard now checks current decoder and timing owners instead of the retired monolithic layout; all 1,053 records and 292 current-gate tests pass. [History](../history/M5-T455-8086-decoder-ledger-guard-reconciliation.md). |
| T454 | Closed: the three fixed-write Console catalog smokes have one CTest-owned build workspace each; serial and repeated parallel replays pass with no source-tree residue. The independent 8086 decoder-ledger guard failure is recorded as CPU debt. [History](../history/M5-T454-parallel-console-profile-smoke-isolation.md). |
| T453 | Closed: fresh default tree uses one WinLibs toolchain family; the one FDC test include is corrected; 44 obsolete build trees are removed; current build operations are reduced to preset-backed commands. [History](../history/M5-T453-toolchain-build-tree-hygiene.md). |
| T452 | Closed: optional isolated ccache presets provide measured repeat-build hits and byte-identical output; default route remains unchanged and task temporary cache/build trees are removed. [History](../history/M5-T452-optional-ccache-build-acceleration.md). |
| T451 | Closed: one current VM request contract, bridge/transport smoke and complete configured build pass; stale request-bridge debt and invalid build instructions are removed without source, CMake or ABI changes. [History](../history/M5-T451-vm-request-bridge-current-build-restoration.md). |
| T450 | Closed: all eight controller pairs are archived with 128 source rows and 128 code-audit rows; S19 verifies the source form of every admitted PDF and preserves all selected-board/personality/media gaps as explicit transfers. [History](../history/M5-T450-core-controller-manuals-and-gap-inventory.md). |
| T449 | Closed: one copied transaction contract, transaction lifecycle, scheduler arbitrator, memory classifier and retirement/time/observation route; all S1--S6 evidence is reconciled and physical/controller/x87 boundaries transfer explicitly. [History](../history/M5-T449-core-cpu-board-transaction-contracts.md). |


## Recent Governance

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
