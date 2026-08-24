# Project Status

## M5 T447 S11 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; `M5 T447 S11`, single-session independent closure audit. |
| Admission And Approval | Accepted S10 P1 `9e05faf8`, the owner-approved T447 proposal, and the owner's standing instruction to continue the dependency-ordered governance closure admit S11 on 2026-08-23. No exception. |
| Objective | Independently reconcile the complete Td S125--S130/T446 closure ledger against current code, build graph, headers, tests, evidence, commits and artifact without making a broader unsupported claim. |
| Non-goals | No new feature, generic framework, duplicate repair path, speculative architectural finding, source/firmware import, or claim of whole-repository perfection. |
| Reference Baseline | Accepted S1--S10 P work, the frozen T447 proposal ledger, Td S125--S130 evidence, current TODO/Queue, and the rebuilt artifact. |
| Candidate Proposal | [M5 Architecture-Boundary Debt Closure](../proposals/m5-architecture-boundary-debt-closure.md), S11. |
| Files And ABI Surface | T447 evidence/status/queue records plus every changed S1--S10 path required for read-only reconciliation. No runtime ABI change is authorized unless an audit proves a current T447 gap that must be repaired. |
| Applicable Rules | Execution P/closure and code-size accounting; architecture one-owner/dependency direction/one production path; coding cohesive owner-local responsibilities; source policy; documentation topology. No exception. |
| Verification | Re-run the frozen row ledger against actual code, build targets, public headers, current tests, committed evidence and artifact; inspect every S1--S10 changed path; record per-row disposition, code-size accounting, focused/full test and gate evidence. |
| Expected Markers | Every ledger row is either accepted with current proof or transferred to an earlier approved receiver with precise reason; no unsupported completion claim; clean artifact and worktree. |
| Asset Needs | Existing non-committed test media only; no new source, firmware, guest-media, or third-party asset. |
| Reporting Requirements | Produce a complete closure-audit evidence record with row-by-row proof, actual caller/source-owner reconciliation, per-S code accounting, final artifact SHA, all verification and diff review. |
| Stop Conditions | Stop for owner direction if a required row has no current evidence and resolving it would expand scope into unapproved implementation, or a claimed row conflicts with a controlling authority. |
| Exit Criteria | The finite T447 ledger is fully reconciled, every S1--S10 P result is accepted or corrected, no remaining T447 TODO/queue receiver remains, and the audit makes no broader claim. |
| Original Owner Request | Execute the approved architecture-boundary closure in dependency order, applying minimalism as a hard constraint rather than a later refactor. |
| Similar-Issue Sweep | Inspect every ledger row and every affected S1--S10 source/build/test/evidence path; distinguish a resolved finding from a related but out-of-ledger concern and record both dispositions. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0447`; `nxvm_0_5_0447.exe` / `build/output/nxvm_0_5_0447.exe`, SHA-256 `8279CA97F977C40EDDF014D03DB21E1A89839E602E81F7D4829D6A14E1BF2601`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T447 S11 | Accepted P1 `4258dbe1`: independently reconciled every finite Td S125--S130/T446 ledger row to current source, guards, evidence and artifact; removed 15 stale resolved TODO entries and made no whole-repository completion claim. [Evidence](../etc/evidence/t447-s11-independent-closure-audit.md). |
| T447 S10 | Accepted P1 `9e05faf8`: `machine.c` is now the 1,090-line Core aggregate/lifecycle coordinator; timing, plan, scheduler, firmware, display, board-device and diagnostic mechanisms each have one owner-local path. The rebuilt artifact, 292 current smokes, 77 specialized gates, documentation governance, exact rollback inventory and actual-diff review pass. [Evidence](../etc/evidence/t447-s10-core-machine-coordinator.md). |
| T447 S9 | Accepted P1 `e44f3369`: removed the VDM session forwarding facade; the machine owner now supplies its direct opaque lifetime contract and composition retains only its input/capture adapter responsibility. The 0447 artifact, 292 current smokes, 77 specialized gates, T345, and documentation governance pass. [Evidence](../etc/evidence/t447-s9-vdm-forwarding-closure.md). |
| T447 S8 | Accepted P1 `acf3f7cc`: all 98 S7 private-layout test consumers have an owner-local or declared-boundary disposition; eighteen VM-composition tests moved unchanged to their owner, and a static gate prevents product/platform regressions. The 0447 artifact, 292 current smokes, 76 specialized gates, exact T345 reconciliation, and documentation governance pass. [Evidence](../etc/evidence/t447-s8-test-boundary-closure.md). |
| T447 S7 | Accepted P1 `b2bf42da`: VM session/media/profile/product layouts are opaque, Console owns its contract while composition performs the binding, and Model-40 has exactly one BYOB construction route. The 0447 artifact, 292 current smokes, 76 specialized gates, T345, and documentation governance pass. [Evidence](../etc/evidence/t447-s7-vm-contract-boundary.md). |
| T447 S5 | Accepted P1 `13f2a5bc` and corrective P2 `af805310`: Core input/presentation and VM platform lifecycle state have one owner-created opaque handle route; stale shape-sensitive gates now prove bounded behavior, and the new S5 guard rejects public-layout regressions. The 0447 artifact, 292 current smokes, 75 specialized gates, T345, and documentation governance pass. [Evidence](../etc/evidence/t447-s5-platform-lifecycle-boundary.md). |
| T447 S6 | Accepted P1 `e342a516`: Core machine collaborators and plan endpoints are opaque, declarative plans are copied, and default PC/AT, Model-339, and Model-40 share bounded creation/rollback operations. The 0447 artifact, 290 current smokes, specialized gates, T345, and documentation governance pass. [Evidence](../etc/evidence/t447-s6-collaborator-plan-boundary.md). |
| T446 | Closed: each VM session owns its debugger cursor state; the sole recorder owner closes on stop, write failure and destruction, clearing failed streams and reporting lifecycle outcomes. [Evidence](../etc/evidence/t446-s1-vm-debugger-recording-lifecycle.md). |
| T445 | Closed: each VM native display adapter now owns its host resource lifecycle; Win32 pairs its DC before window destruction, and Linux curses terminates on the initializing display thread. [Evidence](../etc/evidence/t445-s1-vm-native-display-resource-lifetime.md). |


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
