# Project Status

## Current Work

## M5 T532 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T532's exact App/Core source map and test-owner mirror on 2026-09-20. S1 ledger was committed at `52d2a1a8`; this S executes only its frozen move/delete set. |
| Objective | Relocate the approved NXVM production and repository-only test owners, remove the entire unadmitted VDM skeleton and its consumers, and repair every direct path/build/gate/documentation reference without behavior change. |
| Non-goals | No guest, controller, profile, CLI, UX, shared Lib/Common/x86 API, asset, session-YAML, public C-symbol or CMake-target semantic change; no placeholder directory, forwarding include or compatibility path. |
| Reference Baseline | S1 ledger commit `52d2a1a8`; [ledger](../etc/evidence/t532-s1-single-product-layout-ledger.md). |
| Candidate Proposal | [M5 NXVM single-product layout](../proposals/m5-nxvm-single-product-layout.md), S2. |
| Files And ABI Surface | The finite S1 ledger set: `src/vm`, `src/vdm`, `src/core/machine`, `test/vm`, `test/vdm`, `test/core`, `test/support`, `CMakeLists.txt`, `cmake/verify_*.cmake`, direct includes and current architecture/source-layout documents. |
| Applicable Rules | [Execution](../rules/EXECUTION.md), [Architecture](../rules/ARCHITECTURE.md), [Coding](../rules/CODING.md), [Documentation](../rules/DOCUMENT.md), [System Architecture](../design/ARCHITECTURE.md), [Source Layout](../design/CODING.md), and the S1 ledger. Source policy is not triggered: no source or binary is imported. |
| Verification | Git rename/deletion review; no-former-root and direct-include sweep; CMake configure/build; complete repository-only unit suite; focused VDM-target absence and source-layout/static gates; documentation governance. |
| Expected Markers | `src/{app,core/{core,machine,profile}}`; `test/{app,core/{core,machine,profile}}`; no tracked `src/vm`, `src/vdm`, `test/vm` or `test/vdm`; unchanged external `test/integration`. |
| Asset Needs | None. |
| Reporting Requirements | Record the exact rename/deletion set, actual code/test count change, path sweep, test/build results, VDM deletion proof and any unchanged external-integration status. |
| Stop Conditions | Stop if a reference requires reverse dependency, second composition path, public ABI rename, behavior change, non-ledger destination, or an unresolved non-historical old-path consumer. |
| Exit Criteria | Every ledger item is moved/deleted; direct consumers resolve only final owners; no former root or VDM build/test consumer remains; complete unit and required gates pass; documentation governance passes. |
| Original Owner Request | Reorganize NXVM as one VM product: delete `src/vdm`; move `src/vm/app` to `src/app`, `src/core/machine` to `src/core/core`, `src/vm/machine` to `src/core/machine`, `src/vm/profile` to `src/core/profile`; tests follow the same component ownership except independent integration. |
| Similar-Issue Sweep | Search tracked production source, tests, CMake, static gates, tools and active documents for every former path, `vdm` target and stale owner description. Each hit is moved, deleted, replaced or explicitly retained only as historical evidence. |

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
| T532 S1 | Layout ledger committed at `52d2a1a8`: the owner-approved App/Core relocation, mirrored repository-only-test disposition and VDM deletion set are frozen for S2. |
| T531 | Closed at owner direction on 2026-09-20 after S28 implementation `20c1e71b`: the final canonical SoftPC `2b17749a` six-tree Lib/Common/x86 source-and-test refresh is exact; 338/338 unit, specialized/documentation gates and stripped 0531 x64/x86 artifacts pass. The full external integration remains 41/42: `integration.vm-dos-keyboard-smoke` stays explicitly transferred to [TODO(High)](TODO.md), not relabelled green. [History](../history/M5-T531-shared-common-product-convergence.md). |
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
