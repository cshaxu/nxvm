# Project Status

## Current Work

**Active: M5 T457 S2 - optimized debug artifact and trace audit.**

## M5 T457 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective |
| Admission And Approval | Owner explicitly reopened T457 on 2026-08-24 to govern its artifact performance: every T closure must build its corresponding `build/output/nxvm_0_5_xxxx.exe` as an optimized debug artifact and exclude speed-reducing internal debug logs/traces. |
| Objective | Rebuild T457's current `nxvm_0_5_0457.exe` through one optimized-with-debug-information publication route, and audit every trace/diagnostic hot-path mechanism for release inclusion. |
| Non-goals | Do not change guest timing or device semantics; do not remove a live fault, debugger, trace-provider, or retirement-observation contract without its proven consumer disposition; do not create T458. |
| Reference Baseline | `e9b8fe2d`; `nxvm_0_5_0457.exe` was Debug (`-g`) and the same DOS-prompt smoke measured 4.61 s Debug versus 1.52 s isolated `-O3 -DNDEBUG` Release. |
| Candidate Proposal | [Corrective artifact and trace proposal](../proposals/optimized-artifact-trace-audit.md), limited to the owner-approved scope recorded here; the retained S1 proposal remains historical evidence. |
| Files And ABI Surface | CMake/Preset artifact route, trace/diagnostic source audit, the stale T457 F9 test assertion, focused proof, evidence/history/Current. No public guest or Core ABI unless the audit proves a distinct later S is required. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`; one artifact owner, immutable task identity, one diagnostic owner, and no duplicate runtime route. |
| Verification | Build `vm-0-5-0457` from RelWithDebInfo; prove the artifact compiler flags and version; prove Debug configuration cannot publish it; compare the fixed DOS workload; run trace/diagnostic consumers and the full current gate plus documentation governance. |
| Expected Markers | One optimized-debug current-artifact preset, one build-time non-optimized publication rejection, updated 0457 hash, and a finite trace/diagnostic disposition ledger. |
| Asset Needs | Existing DOS smoke fixture only; no new guest media, third-party source, or external trace. |
| Reporting Requirements | Record compiler flags, artifact hash, Debug/optimized timing comparison, each trace/diagnostic owner and consumer, retained versus excluded status, gates, and any later-S receiver. |
| Stop Conditions | Stop for a required guest-visible timing change, a public Core diagnostic ABI break, a trace consumer outside the audited universe, or a need to discard fault evidence. |
| Exit Criteria | `build/output/nxvm_0_5_0457.exe` is rebuilt only by an optimized-debug route, no inactive logging/trace path is included in it, all live diagnostic mechanisms have an explicit disposition, and required gates pass. |
| Original Owner Request | Use T457 to govern compilation/release performance; every T closes with its optimized debug `build/output` executable and excludes internal debug logging/trace that slows normal execution. |
| Similar-Issue Sweep | Sweep CMake current-artifact and gate presets, all artifact-copy commands, `TYPE_TRACE` macros, Core trace provider calls, per-instruction diagnostics, retirement observation, debugger consumers, and every trace-provider installation. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0457`; `nxvm_0_5_0457.exe` / `build/output/nxvm_0_5_0457.exe`, SHA-256 `D2351D8940209DBB9BAB82FDA0AB33155223F00EDB7B1C74B59B210C855B5E36`. It is built only from the RelWithDebInfo current-artifact route; Debug remains the current-gate route. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T457 | Closed: F9 remains a single host run-handle stop report and no longer enters the guest key route; the two-epoch regression proves second-START `A` reaches KBC. `nxvm_0_5_0457.exe` is built and 293/293 current-gate tests pass. [History](../history/M5-T457-session-input-restart-recovery.md). |
| T456 | Closed at L3: all 18 retained 8259A rows have focused proof or an explicit boundary; one CPU path, programmed cascade, corrected specific-EOI, command/poll lifecycle and default-IR7 acknowledgement remain PIC-owned. Full current-gate is 293/293. PIC-T3 L4 electrical timing is deliberately excluded. [History](../history/M5-T456-core-pic-8259a-phase-contract.md). |
| T455 | Closed: the sole 8086 decoder-ledger guard now checks current decoder and timing owners instead of the retired monolithic layout; all 1,053 records and 292 current-gate tests pass. [History](../history/M5-T455-8086-decoder-ledger-guard-reconciliation.md). |
| T454 | Closed: the three fixed-write Console catalog smokes have one CTest-owned build workspace each; serial and repeated parallel replays pass with no source-tree residue. The independent 8086 decoder-ledger guard failure is recorded as CPU debt. [History](../history/M5-T454-parallel-console-profile-smoke-isolation.md). |
| T453 | Closed: fresh default tree uses one WinLibs toolchain family; the one FDC test include is corrected; 44 obsolete build trees are removed; current build operations are reduced to preset-backed commands. [History](../history/M5-T453-toolchain-build-tree-hygiene.md). |
| T452 | Closed: optional isolated ccache presets provide measured repeat-build hits and byte-identical output; default route remains unchanged and task temporary cache/build trees are removed. [History](../history/M5-T452-optional-ccache-build-acceleration.md). |
| T451 | Closed: one current VM request contract, bridge/transport smoke and complete configured build pass; stale request-bridge debt and invalid build instructions are removed without source, CMake or ABI changes. [History](../history/M5-T451-vm-request-bridge-current-build-restoration.md). |
| T450 | Closed: all eight controller pairs are archived with 128 source rows and 128 code-audit rows; S19 verifies the source form of every admitted PDF and preserves all selected-board/personality/media gaps as explicit transfers. [History](../history/M5-T450-core-controller-manuals-and-gap-inventory.md). |


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
