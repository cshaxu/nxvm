# Project Status

## Current Work

**M5 T439 S1 active -- VM session reset and startup outcome propagation.**

## M5 T439 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved proposal revision and direct admission to S1 in this conversation on 2026-08-23. Scope is the existing VM session reset/startup result path and required FDD initialization status only; no exception is approved. |
| Objective | Consume T438's Core reset-failure contract so session reset/startup and required device initialization return truthful status, stop on failure, and safely retry through their existing paths. |
| Non-goals | No Core reset change, session-manager/public-product contract redesign, second fault or outcome model, profile-policy change, guest-media format change, generic rollback framework, or parallel startup/reset path. |
| Reference Baseline | Pushed `03c08347`; T438 closed; first queued candidate `m5-vm-session-reset-startup-outcomes.md`. |
| Candidate Proposal | [VM session reset and startup outcome propagation](../proposals/m5-vm-session-reset-startup-outcomes.md). |
| Files And ABI Surface | Expected: VM session lifecycle/control/execution callback chain, session device initialization/provider lifecycle, focused VM session smokes and task evidence. Public session-manager and Core public ABI are excluded unless stop conditions apply. |
| Applicable Rules | Architecture: unique mutable outcome owner, one reset/startup production path, and VM-to-Core-only dependency flow. Coding: propagate status at the owning boundary; use existing narrow interfaces; remove discarded status/obsolete void route; no forwarding layer or framework. Execution: complete P, actual-change review, caller sweep, artifact/current-gate and closure evidence. |
| Verification | Focused injected Core-reset failure through `vm_session_start`; failed lifecycle/no-resume/outcome assertion; repaired retry; FDD initialization failure and cleanup; running reset completion outcome; caller sweep; configured build, documentation gate and current gates; artifact identity/hash. |
| Expected Markers | Reset failure reaches the existing session outcome and start caller; session is stopped and does not publish successful start after failure; retry succeeds after repair; FDD initialization failure propagates without a live session; every reset callback/caller has a disposition; no duplicate outcome state, reset route or reverse dependency exists. |
| Asset Needs | None. Use existing project-owned synthetic fixtures and fault injection only; no firmware, guest media, Microsoft component, third-party import or research. |
| Reporting Requirements | Report the single owner/path review, caller-sweep dispositions, focused and current-gate results, artifact hash, code-size delta, all retained live paths, and any transferred issue. |
| Stop Conditions | Stop and request direction if truthful delivery requires public session-manager/product API redesign, changes profile policy or guest-media behavior, requires a second session outcome model, or cannot represent asynchronous reset completion through the existing session boundary. |
| Exit Criteria | Existing reset callback-to-session and device initialization status paths no longer discard errors; start resumes only after successful reset; failure/retry and cleanup proofs pass; review proves unique session outcome ownership, one production path, downward configuration/upward status flow, deleted obsolete truncations, and no needless abstraction; complete P is committed and pushed. |
| Original Owner Request | Owner asked that the next candidate embody hard minimalism, then approved revising its proposal and entering S1 without an intermediate proposal commit. |
| Similar-Issue Sweep | Enumerate every VM session reset callback, execution-context/control/lifecycle reset caller, start/resume caller, and required device initialization status; mark each fixed, intentionally asynchronous through the same outcome, not applicable, or transferred with a receiver. |
## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0439`; `nxvm_0_5_0439.exe` / `build/output/nxvm_0_5_0439.exe`, SHA-256 `CDBA47D65F97C8BD4764BD00F8CF80D19AEE4C7732AADD3BE5E66B0628BE412D`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T438 | Closed: Core is the sole firmware-reset failure owner; discarded firmware-operation errors now return through `core_machine_reset`, leave the machine non-runnable and permit repaired retry. [Evidence](../etc/evidence/t438-s1-core-reset-firmware-failure-atomicity.md). |
| T437 | Closed: the retained [task audit](../history/M5-T437-80386dx-instruction-timing-closure.md) proves all 1,410 legal keys: 1,409 classified CPU retirements and one separate, verified ESC MCP-domain handoff; results, decoder/partition, fresh configuration and 292-test current-gate verification pass. |
| T436 | Closed: [result closure](../etc/cpu-timing/t436-s8-80286-result-closure.md) proves the verified 771-key result set, 8086/80186/80286/80386 regression disposition, current-gate completion and developer artifact. |
| T435 | Stopped by owner-approved requeue; not completed. Its successor 80286 and 80386DX timing closures are now retained as T436 and T437 history. [Record](../history/M5-T435-core-cpu-instruction-timing-program-requeue.md). |
| T434 | Closed: all 30 copied plan declarations/dispositions, every current VM materializer and atomic Core publication route are reconciled; full task closure proof is retained in the [S3 audit](../etc/evidence/t434-s3-task-closure-audit.md) and [task history](../history/M5-T434-core-timing-contract-machine-plan.md). |
| T433 | Closed after corrective S7: 30 frozen Core capabilities have a source-sufficiency and ownership disposition; chip/manual semantics are separated from VM-profile inputs, host/product boundaries and the explicit ATA/HDC source block. [S7 ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md). |
| T432 | Closed: separate Core-private Set-2 translation and typematic break states prevent released keys from restarting typematic; focused regression, Model-339 contract and owner Console/Window verification pass. [Record](../history/M5-T432-keyboard-keyup-reliability-repair.md). |
| T431 | Closed: Model-40 selects a reference-derived unready-drive READ `ST0=48h`/IRQ/result completion through one Core policy; generic PC/AT stays generic, and physical FDC/L3 timing remains transferred. [Evidence](../etc/evidence/t431-s1-deskpro-fdc-not-ready-reference.md). |


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
