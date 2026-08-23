# Project Status

## Current Work

**M5 T440 S1 active -- Model-40 immutable configuration enforcement.**

## M5 T440 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved the first queued Model-40 immutable-configuration candidate for direct admission on 2026-08-23. Scope is profile/session enforcement of the selected DeskPro 386 Model 40 fixed configuration, initially its fixed 1 MiB memory, with no exception. |
| Objective | Prevent generic session configuration mutation from silently changing a materialized Model-40 while retaining legal configurable behavior for profiles that declare it; consolidate the identical private Model-40 fixed-configuration literals into their one owning initializer. |
| Non-goals | No Model-40 hardware-composition change, mutable profile state, Core machine-name branch, public configuration framework, firmware/media policy change or test-only parallel construction path. |
| Reference Baseline | Pushed `05a00edf`; T439 is closed; its recorded developer artifact is the admission baseline. |
| Candidate Proposal | [Model-40 immutable configuration enforcement](../proposals/m5-model40-immutable-configuration.md). |
| Files And ABI Surface | Expected: VM session profile/configuration seam, the two Model-40 constructors' identical private fixed configuration, existing memory reconfiguration entry point, focused Model-40 and generic-session smoke, task evidence and artifact cutover. Core public configuration APIs and product-session public contracts are excluded. |
| Applicable Rules | Architecture: VM profile owns machine selection and its invariants; dependency remains VM to Core. Coding: one profile/session validation owner and one private Model-40 fixed-configuration initializer, status returned to its caller, no parallel configuration route or wrapper. Execution: route sweep, state-unchanged proof, focused and current gates, actual-change review and artifact evidence. |
| Verification | Enumerate Model-40 construction and runtime configuration mutation routes; prove Model-40 rejects an incompatible memory request without changing session/Core state; prove its fixed 1 MiB state; prove a legal generic session reconfiguration remains valid; run focused Model-40 and generic smokes, documentation governance and current gates. |
| Expected Markers | Every in-scope configuration mutation route has a disposition; Model-40 rejection reaches its existing caller, changes neither observed Core memory nor retained session configuration, and adds no Core machine-name condition; generic reconfiguration behavior remains intact. |
| Asset Needs | None. Use existing synthetic Model-40 ROM fixtures only; do not import firmware, guest media, Microsoft components or third-party code. |
| Reporting Requirements | Report configuration-route dispositions, the unique policy owner, Model-40/generic proofs, artifact hash, source/test size delta, retained live routes and any transferred issue. |
| Stop Conditions | Stop and request direction if a correct fixed-profile policy cannot be represented at the existing profile/session seam, requires public API redesign, changes Model-40 hardware composition, or requires Core to identify a machine name. |
| Exit Criteria | All in-scope Model-40 memory mutation routes are rejected or made inapplicable at the VM boundary; failed requests preserve state; the duplicated constructor literal is deleted in favor of one private initializer; legal generic behavior passes; review proves a unique owner and no new configuration framework; complete P is committed and pushed. |
| Original Owner Request | Owner approved this next queued candidate after confirming that Model-40 means Compaq DeskPro 386 Model 40, not IBM 5170. |
| Similar-Issue Sweep | Inventory Model-40 constructor inputs, retained session configuration, runtime reconfiguration APIs, factory/YAML routes and direct Core handoffs; mark each fixed, already profile-fixed, legal generic-only, not applicable or transferred. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0440`; `nxvm_0_5_0440.exe` / `build/output/nxvm_0_5_0440.exe`, SHA-256 `E0A144BC0DBF50F10DB65D718F02485C29BBCA72694F3CA246ACEEC592C69D1F`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T439 | Closed: session reset and startup now return Core failure through one lifecycle outcome; required FDD initialization also propagates status, and no-HDD Model 339 reset avoids an unnecessary unmapped BIOS-table write. [Evidence](../etc/evidence/t439-s1-vm-session-reset-startup-outcomes.md). |
| T438 | Closed: Core is the sole firmware-reset failure owner; discarded firmware-operation errors now return through `core_machine_reset`, leave the machine non-runnable and permit repaired retry. [Evidence](../etc/evidence/t438-s1-core-reset-firmware-failure-atomicity.md). |
| T437 | Closed: the retained [task audit](../history/M5-T437-80386dx-instruction-timing-closure.md) proves all 1,410 legal keys: 1,409 classified CPU retirements and one separate, verified ESC MCP-domain handoff; results, decoder/partition, fresh configuration and 292-test current-gate verification pass. |
| T436 | Closed: [result closure](../etc/cpu-timing/t436-s8-80286-result-closure.md) proves the verified 771-key result set, 8086/80186/80286/80386 regression disposition, current-gate completion and developer artifact. |
| T435 | Stopped by owner-approved requeue; not completed. Its successor 80286 and 80386DX timing closures are now retained as T436 and T437 history. [Record](../history/M5-T435-core-cpu-instruction-timing-program-requeue.md). |
| T434 | Closed: all 30 copied plan declarations/dispositions, every current VM materializer and atomic Core publication route are reconciled; full task closure proof is retained in the [S3 audit](../etc/evidence/t434-s3-task-closure-audit.md) and [task history](../history/M5-T434-core-timing-contract-machine-plan.md). |
| T433 | Closed after corrective S7: 30 frozen Core capabilities have a source-sufficiency and ownership disposition; chip/manual semantics are separated from VM-profile inputs, host/product boundaries and the explicit ATA/HDC source block. [S7 ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md). |
| T432 | Closed: separate Core-private Set-2 translation and typematic break states prevent released keys from restarting typematic; focused regression, Model-339 contract and owner Console/Window verification pass. [Record](../history/M5-T432-keyboard-keyup-reliability-repair.md). |


## Recent Governance

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
