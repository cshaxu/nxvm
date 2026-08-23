# Project Status

## Current Work

## M5 T445 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved the queue-head candidate on 2026-08-23 and required a correct, minimalist repair that reduces unnecessary complexity and redundancy. No exception is allowed. |
| Objective | Give each native display adapter one complete owner-local resource lifecycle: every Win32 `GetDC` is paired with `ReleaseDC`, and Linux curses initialization and `endwin` occur on the display thread. |
| Non-goals | No cross-platform renderer or lifecycle framework; no guest display timing, copied-frame, exclusive-surface, Core, host-support, or public cross-module ABI change; no unrelated display cleanup. |
| Reference Baseline | `master` at `9d5cb849`; current CMake artifact is 0444. `w32adisp` owns the acquired window DC but omits its release, while `linuxcon` initializes curses in its display thread and finalizes it after join in the caller thread. |
| Candidate Proposal | [VM native display resource lifetime repair](../proposals/m5-vm-native-display-resource-lifetime.md). |
| Files And ABI Surface | Expected owners: `src/vm/platform/win32/w32adisp.[ch]`, `src/vm/platform/win32/win32app.c`, `src/vm/platform/linux/linuxcon.c`, existing or narrow adapter-local tests, CMake registration/static checks, artifact and evidence. No public ABI change is expected. |
| Applicable Rules | Architecture: platform adapters own only their host resources and retain one lifecycle path; Coding: repair the repeated mechanism at its owner, reuse current boundaries, add no forwarding framework, and remove obsolete lifecycle state/path; Execution: complete P, similar-issue sweep, artifact and gate closure; Documentation: packet/history/evidence topology. |
| Verification | Freeze the current Win32 and Linux lifecycle paths; add deterministic owner-local seams or focused tests for normal and startup-failure cleanup; statically sweep tracked production and test sources for `GetDC`/`ReleaseDC`, `initscr`/`endwin`, and display-thread finalization; run supported-host builds, relevant focused CTests, `current-fast-smokes-gcc`, `current-gates-gcc`, documentation governance, and a fresh/incremental build comparison. |
| Expected Markers | One `ReleaseDC` pair at the Win32 owning context; `endwin` is reachable only from the initializing Linux display thread; adapter lifecycle regression/static markers pass; current artifact-truth marker for 0445. |
| Asset Needs | None; no imported source, firmware, guest media, or third-party asset. |
| Reporting Requirements | Record each acquisition/finalization owner, success and failure paths, exact similar-issue search/dispositions, supported-host verification boundary, artifact hash, and source/test line accounting in indexed evidence. |
| Stop Conditions | Stop only for an unavailable required native runtime environment after recording the deterministic/static verification boundary; do not replace it with a host-support claim. Any newly found display lifecycle variant is in scope only if it uses the same APIs/owner; otherwise record it for separate admission. |
| Exit Criteria | Every relevant Win32 DC acquisition has its owning release on all completed paths; Linux curses initialization, failure rollback, and normal shutdown finalize on the initializing display thread; no duplicate lifecycle authority or obsolete path remains; focused tests/static sweeps and admitted gates pass; artifact 0445 is built/hashed; evidence and coordinator actual-change review prove the complete sweep. |
| Original Owner Request | Owner approved the first queued task and asked for its brief, one-sentence target, design, and a minimalist repair that reduces complexity and redundant code. |
| Similar-Issue Sweep | Search tracked `src/`, `tests/`, CMake and relevant task records for `GetDC`, `ReleaseDC`, `initscr`, `endwin`, `newterm`, `delscreen`, native renderer creation/finalization, and caller-thread cleanup after adapter-thread initialization; record every production hit as fixed, inapplicable with reason, or separately deferred. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0445`; `nxvm_0_5_0445.exe` / `build/output/nxvm_0_5_0445.exe`, SHA-256 `2D369C04936536F987ECD59826037949E70DF6BCC33241C131530E4BA724808D`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T444 | Closed: the 20 fast-smoke fixture failures use the legal 16-byte reset window; T344 classifies four manifest producers separately from 71 historical fixtures; every discovered stale gate now proves the current single Core plan route. [Evidence](../etc/evidence/t444-s1-current-gate-regression-restoration.md). |
| T443 | Closed: the mailbox C11-initializes its sole lock, every production renderer stops on failed capture, and VM display generation commits only after one accepted publication path. [Evidence](../etc/evidence/t443-s1-core-platform-primitive-outcomes.md). |
| T442 | Closed: one Core lexical owner keeps 8086 `0F` primary, rejects it on 80186, and consumes it as extended on 80286/80386; one RAM mapping owner rejects a 32-bit physical span overflow before publishing state. [Evidence](../etc/evidence/t442-s1-core-cpu-memory-boundary-correctness.md). |
| T441 | Closed: VM media saving has one staging owner; FDD now releases derived sidecar state on failed paired persistence, and focused failure smoke proves resident/file/temporary preservation. [Evidence](../etc/evidence/t441-s1-vm-media-persistence-failure-hygiene.md). |
| T440 | Closed: Model-40 configuration has one private initializer; incompatible creation/runtime memory changes are rejected at the VM boundary, with generic session reconfiguration retained. [Evidence](../etc/evidence/t440-s1-model40-immutable-configuration.md). |
| T439 | Closed: session reset and startup now return Core failure through one lifecycle outcome; required FDD initialization also propagates status, and no-HDD Model 339 reset avoids an unnecessary unmapped BIOS-table write. [Evidence](../etc/evidence/t439-s1-vm-session-reset-startup-outcomes.md). |
| T438 | Closed: Core is the sole firmware-reset failure owner; discarded firmware-operation errors now return through `core_machine_reset`, leave the machine non-runnable and permit repaired retry. [Evidence](../etc/evidence/t438-s1-core-reset-firmware-failure-atomicity.md). |
| T437 | Closed: the retained [task audit](../history/M5-T437-80386dx-instruction-timing-closure.md) proves all 1,410 legal keys: 1,409 classified CPU retirements and one separate, verified ESC MCP-domain handoff; results, decoder/partition, fresh configuration and 292-test current-gate verification pass. |


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
