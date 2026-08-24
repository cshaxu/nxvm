# Project Status

## M5 T447 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; `M5 T447 S5`, single-session coordinator/executor followed by coordinator closure review. |
| Admission And Approval | The owner-approved T447 proposal and accepted S4 P1 `38f5097e` admit the next dependency-ordered S5 batch on 2026-08-23. No exception. |
| Objective | Seal Core platform objects and VM platform lifecycle state behind owner-created opaque handles and bounded operations, preserving host behavior. |
| Non-goals | No generic host framework, host-policy move into Core, product UX change, new asynchronous contract, session/profile redesign, media-format change, or unrelated cross-owner-test migration. |
| Reference Baseline | Accepted S4 P1 `38f5097e`; Td S125 E, Td S126 A, T445, and T447 proposal S5. |
| Candidate Proposal | [M5 Architecture-Boundary Debt Closure](../proposals/m5-architecture-boundary-debt-closure.md), S5. |
| Files And ABI Surface | Core input-source and presentation-mailbox contracts/implementations; VM composition consumers; VM platform run, host-surface, request transport, execution, virtual-time contracts/implementations and focused tests; CMake gates; T447 history/evidence/status and current artifact. |
| Applicable Rules | Execution P/closure and code-size requirements; architecture single-state-owner, Core-directed dependencies, opaque public contracts and composition-only integration; coding minimal durable capabilities with no forwarding wrappers; documentation topology. No exception. |
| Verification | Inventory every public mutable layout, direct field access, construction/finalize path, and host callback in the named Core/VM surfaces; prove each replacement has one owner and every caller uses bounded operations or copied observations; run focused lifecycle/platform failure tests, static boundary proof, T345, current artifact/smokes, specialized gates, and documentation governance. |
| Expected Markers | Retained T445/platform markers plus T447 S5 marker; no public mutable Core platform or VM run/host-surface/request/execution/virtual-time layout, no reverse dependency, no duplicate lifecycle route, and no changed host lifetime behavior. |
| Asset Needs | Existing non-committed test media only; no new source, firmware, guest-media, or third-party asset. |
| Reporting Requirements | Record complete layout/caller/lifecycle inventory, selected owner and failure semantics, deleted layouts/routes, code-size accounting, artifact SHA, focused/full verification, and actual-diff review; commit and push one complete implementation P before coordinator closure. |
| Stop Conditions | Stop for owner direction if closure requires a new asynchronous host contract, changes Linux/Win32 resource lifetime or product-visible behavior, needs a test-only public seam, or cannot preserve copied-frame and input-cancellation behavior. |
| Exit Criteria | Every named Core/VM stateful contract is opaque or owner-allocated; all consumers use bounded lifecycle/operation paths and copied frames; Linux/Win32 lifetime proof remains valid; no compatibility layout or parallel lifecycle path remains. |
| Original Owner Request | Execute the approved architecture-boundary closure in dependency order, applying minimalism as a hard constraint rather than a later refactor. |
| Similar-Issue Sweep | Inspect every Core platform stateful header/source, VM composition/platform/public header and CMake consumer, Linux/Win32 adapter, direct field/callback access, platform and integration test, T345 entry, and prior T445/T443 evidence; do not repair only the first exposed layout. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0447`; `nxvm_0_5_0447.exe` / `build/output/nxvm_0_5_0447.exe`, SHA-256 `A38E741BE21C3E4C4E5D3E0802694D64D0221BF114D960A53CF68C61BC82E852`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T447 S4 | Accepted P1: one Core-owned synchronous file route now serves named VM consumers; the unused callback route and recorder source-build exception are deleted. The current 0447 artifact, 292 current smokes, and 75 specialized gates pass. [Evidence](../etc/evidence/t447-s4-file-resource-boundary.md). |
| T446 | Closed: each VM session owns its debugger cursor state; the sole recorder owner closes on stop, write failure and destruction, clearing failed streams and reporting lifecycle outcomes. [Evidence](../etc/evidence/t446-s1-vm-debugger-recording-lifecycle.md). |
| T445 | Closed: each VM native display adapter now owns its host resource lifecycle; Win32 pairs its DC before window destruction, and Linux curses terminates on the initializing display thread. [Evidence](../etc/evidence/t445-s1-vm-native-display-resource-lifetime.md). |
| T444 | Closed: the 20 fast-smoke fixture failures use the legal 16-byte reset window; T344 classifies four manifest producers separately from 71 historical fixtures; every discovered stale gate now proves the current single Core plan route. [Evidence](../etc/evidence/t444-s1-current-gate-regression-restoration.md). |
| T443 | Closed: the mailbox C11-initializes its sole lock, every production renderer stops on failed capture, and VM display generation commits only after one accepted publication path. [Evidence](../etc/evidence/t443-s1-core-platform-primitive-outcomes.md). |
| T442 | Closed: one Core lexical owner keeps 8086 `0F` primary, rejects it on 80186, and consumes it as extended on 80286/80386; one RAM mapping owner rejects a 32-bit physical span overflow before publishing state. [Evidence](../etc/evidence/t442-s1-core-cpu-memory-boundary-correctness.md). |
| T441 | Closed: VM media saving has one staging owner; FDD now releases derived sidecar state on failed paired persistence, and focused failure smoke proves resident/file/temporary preservation. [Evidence](../etc/evidence/t441-s1-vm-media-persistence-failure-hygiene.md). |
| T440 | Closed: Model-40 configuration has one private initializer; incompatible creation/runtime memory changes are rejected at the VM boundary, with generic session reconfiguration retained. [Evidence](../etc/evidence/t440-s1-model40-immutable-configuration.md). |
| T439 | Closed: session reset and startup now return Core failure through one lifecycle outcome; required FDD initialization also propagates status, and no-HDD Model 339 reset avoids an unnecessary unmapped BIOS-table write. [Evidence](../etc/evidence/t439-s1-vm-session-reset-startup-outcomes.md). |


## Recent Governance

- **M5 Td S134 P1:** created and placed first the coverage-bearing
  [architecture-boundary debt closure proposal](../proposals/m5-architecture-boundary-debt-closure.md).
  It maps every Td S125--S130 finding and the T446 direct-test exception to
  bounded dependency-ordered S work; documentation governance passes. This
  creates no numeric task or runtime change.

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
