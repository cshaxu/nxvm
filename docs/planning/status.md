# Project Status

## Current Work

**M5 T291 S4: Protected-return fault atomicity -- active.**

| Requirement | Acceptance evidence |
| --- | --- |
| Preserve the architectural state of a faulting protected-mode far return. | A focused core corpus proves invalid or non-present return `CS`/`SS` and an inaccessible return entry deliver the required fault without a partial `SP`, `CS`, `SS`, `IP`, `CPL`, segment-cache, or flags commit. |
| Close the equivalent protected-mode `RETF` and `IRET` defect class within the current implementation. | The similar-issue sweep records every relevant pop-before-validation path in `cpu_instructions.c`, fixes each in scope or records a bounded deferral with owner approval. |
| Retain successful privilege-return behavior. | Existing protected privilege and call-gate corpora pass alongside the new negative corpus; full `current-gates-gcc` passes. |

- **Original request:** append a tracked subtask to repair the audit finding
  that protected `RETF`/`IRET` may advance the guest stack before all
  return-frame validation succeeds. The owner corrected the initial T287
  allocation to the current T291 sequence; this is S4.
- **Owner-approved identity exception:** T291 S4 is a one-time late subtask of
  the existing T291 allocation, admitted after T291 S3 because concurrent
  queue allocation had already consumed the intended identifier. It allocates
  no second numeric task. If it changes runtime behavior, it rebuilds the same
  `0.5.0291` task artifact and records its source commit and replacement SHA;
  the T291 S3 artifact evidence remains in history. This exception does not
  permit any future closed task identifier to be reused.
- **Owner-approved governance exception:** M5 Td S42 may update only the
  numbered-queue and contract wording while this packet is active. It must not
  alter this task's implementation, tests, scope, or acceptance evidence.
- **Scope:** the protected-mode far-return and outer-privilege `IRET` paths,
  their smallest shared non-mutating frame-read/preflight support if needed,
  and focused core tests/CMake registration.
- **Non-goals:** new instruction families, 32-bit protected-return completion,
  task switching, new Windows compatibility claims, or a broad CPU refactor.
- **Risk:** the current path is live in call-gate and protected-privilege
  handling; changing its ordering can regress valid returns or fault delivery.
  Keep the commit boundary small and verify both valid and failing frames.
- **Similar-issue sweep:** before changing code, inspect all protected return
  paths reached through `_e_ret_far`, `_ser_ret_far_outer`,
  `_ser_iret_protected_outer_16`, and their direct helpers. Record each
  production hit as fixed, not applicable with a reason, or deferred in
  `TODO.md`; do not broaden the task to unrelated ordinary `POP` operations.
- **Rules:** Architecture overview, module layout, contracts, coding standard,
  source policy, execution workflow, and execution policy apply. No exception
  is requested.
- **Verification:** run the new focused corpus, retained
  `current.core-machine-protected-privilege-smoke` and
  `current.core-machine-call-gate-smoke`, then `current-gates-gcc`. On closure,
  record the sweep, commands, exact markers, commit, and the task's rebuilt
  `0.5.0291` developer artifact identity as required by the task-ID policy.
- **Stop condition:** all stated failure cases are atomic at the defined guest
  boundary, successful returns remain covered, and no in-scope production hit
  lacks a recorded disposition.

## Current Technical Baseline

- **T291 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0291`. Artifact
  `nxvm_0_5_0291.exe` SHA-256:
  `9CD9912B27D9B3FB199480788E5E21E6A490904C9C4C751FE31D2224961E878A`.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, the current
  post-`#UD` transition pending T293 removal, immutable ROM mapping, and
  validated real-mode entry plans.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T285 | Implemented bounded EGA mode 10h direct, turned the T284 core/VM corpus into normal success coverage, and emitted the 0.5.0285 developer artifact. |
| T286 | Fixed the corpus-proven ATA device-control `nIEN` IRQ14 visibility gap through core-owned controller state, with core, VM-port, and guest fixture success evidence; no DMA, timing, or command expansion. |
| T287 | Fixed bounded ROM CHS device/head, AH=08h caller-pointer, FDISK text-service/window-clear, and HDD-only boot-selection defects; external DOS registers C:, presents stable FDISK copied frames, and hands off from HDD-only ATA boot, while the Standard-mode checkpoint remains a research result, not a Windows support claim. |
| T288 | Resolved the external Setup post-copy `#CE` with the bounded 16-bit protected-mode call-gate and outer-`RETF` path; core and real replay now reach the next `MOV CR0,EAX` `#UD` checkpoint without claiming Windows support. |
| T289 | Materialized the default PC/AT ROM image before provider freeze, including its A20 reset alias, while reset restores only IVT/BDA and mutable device tables; ROM, boot, Console, debugger, and display regressions pass. |
| T290 | Replaced the FDC single-media binding with frozen drive slots and exact DOR/unit selection, proven through core ports, VM composition, and DOS FDD0 regression without broadening FDC behavior. |
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S32:** admitted the original second core/composition migration.
- **M5 Td S33:** corrected the external-consumer boundary and shifted the
  former forward queue to T279--T287.
- **M5 Td S34:** repaired queue wording and inserted the approved T279--T283
  reliability/interface package.
- **M5 Td S35:** tightened T279, T280, T282, and T283 task contracts.
- **M5 Td S36:** recorded the WASM media-backing admission boundary.
- **M5 Td S37:** made M5 history static, capped closure summaries, and
  hardened the mojibake governance check.
- **M5 Td S40:** inserted the approved T293--T303 core public-surface closure
  and shifted remaining Windows CPU/FPU/readiness/closure work to T304--T308.
- **M5 Td S41:** retired completed planning detail, made TODO open-only, removed
  tracked local-path samples, and added recurrence gates for those boundaries.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Project Status](status.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
