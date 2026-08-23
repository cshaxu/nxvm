# Project Status

## Current Work

## M5 T446 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; `M5 T446 S1`, single-session coordinator/executor then coordinator closure review. |
| Admission And Approval | Owner approval: active thread goal, 2026-08-23, single-agent minimalist completion of VM debugger-state and recording-lifecycle repair. Admits Queue head only; no exception. |
| Objective | Make debugger cursor state session-instance-owned and make recorder start/write/stop/finalize outcomes truthful at the existing VM debug boundary. |
| Non-goals | No Core debugger redesign, command-interpreter replacement, console command ABI change, trace-format change, asynchronous recording, or new abstraction layer. |
| Reference Baseline | `5f6256e6` prior T445 source commit; Queue head proposal. |
| Candidate Proposal | [M5 VM debugger state and recording lifecycle repair](../proposals/m5-vm-debugger-recording-lifecycle.md). |
| Files And ABI Surface | `src/vm/machine/debug.[ch]`, VM session lifecycle/control/console adapter only if needed, focused smoke/CMake registration, current artifact/version, evidence/history/status. Preserve retained console commands and record format. |
| Applicable Rules | `docs/rules/EXECUTION.md`: packet, P lifecycle, similar sweep, artifact, closure; architecture: one mutable-state owner/production path and `vm -> core`; coding: owner-local cohesive repair, no forwarding facade, remove obsolete path; documentation topology/index rules. No exception. |
| Verification | New focused two-session cursor isolation plus recorder start/write/stop/finalize-failure smoke; complete recorder-call sweep; static recurrence gate when mechanically feasible; `current-fast-smokes-gcc`, `current-gates-gcc`, documentation governance; fresh exact-source `vm-0-5-0446` build and SHA-256. |
| Expected Markers | `M5:T446:S1:VM-DEBUGGER-RECORDING-LIFECYCLE:OK`; static gate marker; all current gates pass. |
| Asset Needs | None; no firmware, guest media, trace fixture, or external source. |
| Reporting Requirements | Report admission/brief confirmation, implementation P1 pushed, coordinator actual-change review, P2 closure pushed; evidence maps every proposal requirement, call-site disposition, source line accounting, artifact identity/SHA, and retained boundary. |
| Stop Conditions | Stop and obtain owner direction only if making failures observable requires a console command-status ABI change or a Core debugger-context redesign. |
| Exit Criteria | Each session owns its cursor state; all start/write/stop/finalize recorder failures leave no false active state and are observable at the existing VM debug boundary; destruction closes once; every recorder caller is disposed; focused tests/current gates/fresh artifact pass; actual-diff review finds no duplicate lifecycle path. |
| Original Owner Request | Single-agent minimalist completion of VM debugger-state and recording-lifecycle repair. |
| Similar-Issue Sweep | Search all tracked production, tests, CMake and governance records for `recordFile`, `vm_machine_debug_record_`, `STD_FOPEN`, `STD_FCLOSE`, `STD_FPRINTF` and debugger cursor/context state. Classify every production hit; repair the shared VM debug owner or transfer any out-of-scope hit. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0446`; `nxvm_0_5_0446.exe` / `build/output/nxvm_0_5_0446.exe`, SHA-256 `2265CC4B0182EF66063327FC55118BAA44C95F270B7B0680131E8813EA412CBF`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T445 | Closed: each VM native display adapter now owns its host resource lifecycle; Win32 pairs its DC before window destruction, and Linux curses terminates on the initializing display thread. [Evidence](../etc/evidence/t445-s1-vm-native-display-resource-lifetime.md). |
| T444 | Closed: the 20 fast-smoke fixture failures use the legal 16-byte reset window; T344 classifies four manifest producers separately from 71 historical fixtures; every discovered stale gate now proves the current single Core plan route. [Evidence](../etc/evidence/t444-s1-current-gate-regression-restoration.md). |
| T443 | Closed: the mailbox C11-initializes its sole lock, every production renderer stops on failed capture, and VM display generation commits only after one accepted publication path. [Evidence](../etc/evidence/t443-s1-core-platform-primitive-outcomes.md). |
| T442 | Closed: one Core lexical owner keeps 8086 `0F` primary, rejects it on 80186, and consumes it as extended on 80286/80386; one RAM mapping owner rejects a 32-bit physical span overflow before publishing state. [Evidence](../etc/evidence/t442-s1-core-cpu-memory-boundary-correctness.md). |
| T441 | Closed: VM media saving has one staging owner; FDD now releases derived sidecar state on failed paired persistence, and focused failure smoke proves resident/file/temporary preservation. [Evidence](../etc/evidence/t441-s1-vm-media-persistence-failure-hygiene.md). |
| T440 | Closed: Model-40 configuration has one private initializer; incompatible creation/runtime memory changes are rejected at the VM boundary, with generic session reconfiguration retained. [Evidence](../etc/evidence/t440-s1-model40-immutable-configuration.md). |
| T439 | Closed: session reset and startup now return Core failure through one lifecycle outcome; required FDD initialization also propagates status, and no-HDD Model 339 reset avoids an unnecessary unmapped BIOS-table write. [Evidence](../etc/evidence/t439-s1-vm-session-reset-startup-outcomes.md). |
| T438 | Closed: Core is the sole firmware-reset failure owner; discarded firmware-operation errors now return through `core_machine_reset`, leave the machine non-runnable and permit repaired retry. [Evidence](../etc/evidence/t438-s1-core-reset-firmware-failure-atomicity.md). |


## Recent Governance

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
