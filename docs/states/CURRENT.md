# Project Status

## Current Work

**Active: M5 T441 S1.**

## M5 T441 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved the first queued candidate on 2026-08-23 and required the implementation to minimize logic and code. This admits the linked media-persistence proposal and one bounded S1 implementation. |
| Objective | Make VM-owned removable-media pair saving and HDD single-file saving return failure without leaked temporary/sidecar allocations or falsely reporting a completed paired save; retain one owner-local staging/commit/rollback path. |
| Non-goals | No new media format, filesystem layer, asynchronous I/O, cross-file atomicity claim beyond current host rename semantics, Core change, physical-media semantics, or unrelated session storage cleanup. |
| Reference Baseline | `master` at `de93c6ba`; prior current developer artifact; proposal `m5-vm-media-persistence-failure-hygiene.md`; existing `vm_machine_media_save_atomically` and `vm_machine_media_save_pair_atomically` owners. |
| Candidate Proposal | [VM media persistence failure hygiene](../proposals/m5-vm-media-persistence-failure-hygiene.md); task record [T441](../history/M5-T441-vm-media-persistence-failure-hygiene.md). |
| Files And ABI Surface | Expected private VM-machine save owner, FDD caller, focused VM media smoke, build artifact identity and task evidence/status. Existing `vm_machine_media_save_*` interface may be reduced only if all callers remain covered; no new public cross-module ABI. |
| Applicable Rules | Architecture: one mutable persistence/failure owner and one production path; Coding: owner-local helper only when it deletes duplication, no forwarding layer or unused compatibility route; Execution: mechanism-defect shared-owner/caller sweep, focused proof, artifact and actual-change review; Documentation: indexed evidence/history/current-status boundaries. |
| Verification | Enumerate all `vm_machine_media_save_*` callers; add deterministic focused coverage for FDD serialization/allocation and paired staging/replacement/rollback cleanup plus HDD single-save failure preservation; build/run affected smoke, fast current smokes, documentation governance, and record any independent full-gate baseline. |
| Expected Markers | Existing `M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK`, `M5:T280:S2:ATOMIC-MEDIA:OK`, `M5:T283:S6:ATOMIC-SAVE:OK`; a new T441 failure-hygiene marker only if a focused executable assertion needs one. |
| Asset Needs | None; synthetic VM media and repository-owned temporary test files only. |
| Reporting Requirements | Evidence must identify the shared save owner, every FDD/HDD caller, each allocation/temp/replacement/rollback stage, cleanup result, source/test line delta and the no-stronger-than-host-rename boundary. |
| Stop Conditions | Stop for owner direction if portable behavior cannot preserve both prior files after a cross-file replacement failure, if failure injection requires a new broad test-only or public filesystem abstraction, or if an external filesystem policy decision is required. |
| Exit Criteria | One minimal save mechanism owns staging and cleanup; FDD/HDD callers have one observable failure path; injected failure stages prove retained state and no owned temporary/sidecar allocation/file tail; caller sweep is complete; focused and applicable gates pass; a `0.5.0441` artifact, evidence, review and closure records are pushed. |
| Original Owner Request | Implement the next queued task while keeping logic and code as small as possible. |
| Similar-Issue Sweep | Search every tracked production/test caller of `vm_machine_media_save_atomically` and `vm_machine_media_save_pair_atomically`, plus direct FDD sidecar serialization. Dispose each as corrected, covered by the shared owner, or explicitly out of scope with reason. |


## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0441`; `nxvm_0_5_0441.exe` / `build/output/nxvm_0_5_0441.exe`, SHA-256 `C06CF7C46553E53FE7592ABF9C7900110DB3F6A1FF4183C300F4EC8B4D40273F`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T440 | Closed: Model-40 configuration has one private initializer; incompatible creation/runtime memory changes are rejected at the VM boundary, with generic session reconfiguration retained. [Evidence](../etc/evidence/t440-s1-model40-immutable-configuration.md). |
| T439 | Closed: session reset and startup now return Core failure through one lifecycle outcome; required FDD initialization also propagates status, and no-HDD Model 339 reset avoids an unnecessary unmapped BIOS-table write. [Evidence](../etc/evidence/t439-s1-vm-session-reset-startup-outcomes.md). |
| T438 | Closed: Core is the sole firmware-reset failure owner; discarded firmware-operation errors now return through `core_machine_reset`, leave the machine non-runnable and permit repaired retry. [Evidence](../etc/evidence/t438-s1-core-reset-firmware-failure-atomicity.md). |
| T437 | Closed: the retained [task audit](../history/M5-T437-80386dx-instruction-timing-closure.md) proves all 1,410 legal keys: 1,409 classified CPU retirements and one separate, verified ESC MCP-domain handoff; results, decoder/partition, fresh configuration and 292-test current-gate verification pass. |
| T436 | Closed: [result closure](../etc/cpu-timing/t436-s8-80286-result-closure.md) proves the verified 771-key result set, 8086/80186/80286/80386 regression disposition, current-gate completion and developer artifact. |
| T435 | Stopped by owner-approved requeue; not completed. Its successor 80286 and 80386DX timing closures are now retained as T436 and T437 history. [Record](../history/M5-T435-core-cpu-instruction-timing-program-requeue.md). |
| T434 | Closed: all 30 copied plan declarations/dispositions, every current VM materializer and atomic Core publication route are reconciled; full task closure proof is retained in the [S3 audit](../etc/evidence/t434-s3-task-closure-audit.md) and [task history](../history/M5-T434-core-timing-contract-machine-plan.md). |
| T433 | Closed after corrective S7: 30 frozen Core capabilities have a source-sufficiency and ownership disposition; chip/manual semantics are separated from VM-profile inputs, host/product boundaries and the explicit ATA/HDC source block. [S7 ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md). |


## Recent Governance

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
