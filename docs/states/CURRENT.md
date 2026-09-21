# Project Status

## Current Work

## M5 T531 S28 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner accepted S27 and approved S28 on 2026-09-20: import and adapt the recently updated SoftPC Common, Lib and x86 source/test corpus; compile, test, commit and push, then await owner validation. T531 remains open. |
| Objective | Replace NXVM's six shared trees byte-for-byte with SoftPC `2b17749a12c1132d9c9c65754008befbcb764546`, then directly adapt NXVM to the refreshed public contracts. |
| Non-goals | No SoftPC write, no NXVM-specific code in the six shared trees, no Core/controller semantic change, no compatibility aliases, and no asset/session-YAML edit. |
| Reference Baseline | NXVM `a10445eb`; committed SoftPC `2b17749a12c1132d9c9c65754008befbcb764546` with no uncommitted changes in the six imported trees; accepted S27 is the immediate predecessor. |
| Candidate Proposal | [M5 shared Common product convergence](../proposals/m5-shared-common-product-convergence.md), S28. |
| Files And ABI Surface | Complete `src/lib`, `src/common`, `src/x86`, `test/lib`, `test/common`, and `test/x86` imports; NXVM App/VM/CMake consumer adaptation; task evidence and task history. |
| Applicable Rules | [Execution](../rules/EXECUTION.md), [Architecture](../rules/ARCHITECTURE.md), [Coding](../rules/CODING.md), [Documentation](../rules/DOCUMENT.md), [System Architecture](../design/ARCHITECTURE.md), [Source Layout](../design/CODING.md), and the source policy. Lib/Common remain neutral; x86 is an explicitly selected architecture-specific frontend. |
| Verification | Before/after six-tree corpus diff; all manifest/dependency gates; direct NXVM debugger/assembler tests; retired-path scan; x64/x86 Release builds; complete unit suite; external integration; actual-diff review and documentation governance. |
| Expected Markers | SoftPC revision `2b17749a`; exact equality of all six trees; Common has no x86 dependency; NXVM selects x86 only through public interfaces; no retired Common Debug/xasm32 route. |
| Asset Needs | None; repository-only corpus tests only. External integration uses established owner-managed assets without modifying them. |
| Reporting Requirements | Record frozen source revision, six-tree identity, consumer adaptations, verification/build outcomes, line delta, and every remaining non-green integration row without attribution. |
| Stop Conditions | Stop for incompatible public contract requiring a new product-policy seam, required Core semantic change, source-license conflict, or adapter-boundary regression that cannot be repaired without a shared-tree fork. |
| Exit Criteria | Six trees equal frozen SoftPC; NXVM consumes the public x86/Common/Lib contracts through one path; all required builds/tests and documentation governance pass; actual review proves no compatibility branch. Deliver a pushed P commit and await owner validation; T531 remains open for its whole-task two-product execution gate. |
| Original Owner Request | Close S27, then import the recently updated upstream SoftPC Common, Lib and x86 source/test six-component corpus verbatim, adapt NXVM, compile/test/commit/push, and await validation. |
| Similar-Issue Sweep | Inspect every shared public-contract include, build target, test registration, manifest, and NXVM command integration changed by the refresh; each must use the refreshed contract or be removed, with no parallel frontend path. |

## Current Technical Baseline

- `vm-0-5-0531` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0531_x64.exe` and `nxvm_0_5_0531_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, and `ui`. The separately selected `src/x86` corpus owns
  `xasm32` and the x86 Debug CLI; Common has no x86 dependency.
- S21 P1 `b35357ca` and P2 `e0d3b946` established the initial NXVM diagnosis.
  S22 replaces their private Common/Lib hunks with SoftPC's canonical corrected
  corpus and retains only the NXVM VADP adapter concern where still needed.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T531 S27 | Accepted at `a10445eb`: imported exact SoftPC `f6dadddd` Lib/Common/x86 source and tests, adapted NXVM to the public x86 split, and proved 337/337 unit plus dual stripped 0531 artifacts. The retained external keyboard-smoke timeout is recorded without attribution in [evidence](../etc/evidence/t531-s27-six-package-x86-import.md); S28 refreshes the same corpus while T531 remains open. |
| T531 S26 | Accepted at `0840a34b`: imported exact SoftPC `95c467a7` Lib/Common source and tests; added one per-media YAML Storage mode (`direct`, `readonly`, `overlay`, default overlay) through App, Common and VM-machine with no parallel opener. [Evidence](../etc/evidence/t531-s26-modeful-media-import.md). |
| T531 S11 | Accepted at `3a275380`: imported the exact 94-file SoftPC `987d82e` KVM Lib corpus, deleted all retired UI component paths, and directly cut Common, VM, CMake and tests to the canonical contracts. Lib boundaries, x64/x86 optimized 0531 builds, 299/299 unit and documentation governance pass. [Closure evidence](../etc/evidence/t531-s11-canonical-kvm-lib-refresh.md). |
| T530 | Closed at owner direction on 2026-09-13 after implementation bc9a1247; pinned canonical Lib import and Common adaptation, recorded 299/299 unit and 42/42 integration, both 0530 artifacts verified by hash. No new manual test is claimed. Later source revisions and Common UX convergence belong to the next task. [History](../history/M5-T530-canonical-softpc-lib-refresh.md). |
| T529 | Closed at `cc5fd4a7`: S1 exhaustively classified the 29-file VM-machine corpus and S2 removed the only two proven dead controls. No Common migration was invented: Common remains the sole copied protocol owner; Core and CCPU executors remain distinct product adapters. 299/299 unit, focused regression, specialized/documentation gates and stripped 0528 x64/x86 artifacts pass. The unchanged 39/42 external-integration exception remains transferred to the Model-40/5170 boot-chain TODO. [History](../history/M5-T529-common-machine-residual-audit.md) and [evidence](../etc/evidence/t529-s2-dead-control-cleanup.md). |
| T528 | Closed under the owner's 2026-09-12 exception after `9d892446`: all VM-machine duplicate owners are removed, 299 unit cases, specialized gates, documentation governance and stripped x64/x86 artifacts pass. The known 39/42 external-integration result is transferred unchanged to the external-ROM Model-40/IBM-5170 boot-chain TODO; it is not represented as green. [History](../history/M5-T528-vm-machine-owner-cleanup.md) and [evidence](../etc/evidence/t528-s7-task-closure-attempt.md). |
| T527 | Common product-runtime convergence closed at `2c596f2c`: one Common corpus owns xasm32, Debug, session, machine and UI; NXVM retains one VM Core adapter, product policy owner and App composition root. Independent Common verification, 299/299 unit, 42/42 external integration, specialized gates and stripped x64/x86 artifacts pass. [History](../history/M5-T527-common-product-runtime-convergence.md) and [evidence](../etc/evidence/t527-s9-reusable-common-closure.md). |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 Td S166 P1:** queued two owner-approved, unnumbered candidates in
  dependency order: VM-machine owner cleanup, then Common-machine executor
  completion. Both use the audited NXVM/SoftPC two-consumer criterion; neither
  allocates or admits a numeric implementation task.

- **M5 T527 S9 P2:** records independent corpus verification, complete test
  closure, artifact identity, proposal retention, and task closure after
  implementation `2c596f2c`.
