# Project Status

## Current Work

## M5 T532 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved the complete single-product App/Core source map and matching repository-only test layout; S2 was accepted at `e9c5a55f`. This S completes only the proposal's defined closure sweep. |
| Objective | Prove the final App/Core/test owner map, build the T532 dual stripped Release artifacts, run complete repository-only unit and independent integration verification, and record the truthful final task disposition. |
| Non-goals | No functional guest, controller, profile, CLI, UX, Lib/Common/x86 API, external asset, session-YAML or integration-test semantic change; no repair of an unrelated external-integration failure merely to make this layout task green. |
| Reference Baseline | Accepted S2 implementation `66bba6f3`, acceptance `e9c5a55f`, and [S2 evidence](../etc/evidence/t532-s2-single-product-layout-relocation.md). |
| Candidate Proposal | [M5 NXVM single-product layout](../proposals/m5-nxvm-single-product-layout.md), S3. |
| Files And ABI Surface | Build-version/artifact declarations, current technical baseline, T532 history/evidence, and any mechanically stale source/test/CMake/static-gate path discovered by the defined sweep. |
| Applicable Rules | [Execution](../rules/EXECUTION.md), [Architecture](../rules/ARCHITECTURE.md), [Coding](../rules/CODING.md), [Documentation](../rules/DOCUMENT.md), [System Architecture](../design/ARCHITECTURE.md), and [Source Layout](../design/CODING.md). Source policy is not triggered: no source or binary is imported. |
| Verification | Final tracked-path and CMake/include sweep; clean x64 and x86 Release configurations and stripped artifacts; complete repository-only unit suite; independent owner-managed integration suite; specialized/documentation gates; actual-change and task-closure review. |
| Expected Markers | No tracked `src/vm`, `src/vdm`, `test/vm` or `test/vdm`; final `src/{app,core/{core,machine,profile}}` and mirrored tests; `nxvm_0_5_0532_{x64,x86}.exe` in `build/output` and `assets/sessions`; truthful integration result. |
| Asset Needs | Existing owner-managed integration assets only; no asset creation, import, copying or configuration mutation. |
| Reporting Requirements | Record exact path-sweep result, x64/x86 artifact hashes and PE architectures, complete unit/integration results, all gate outcomes, actual code-size/path review, and any transferred external-integration result. |
| Stop Conditions | Stop and report if closure needs a behavior change, external asset change, a new compatibility root, an unresolved old-path consumer, or an unapproved integration repair. |
| Exit Criteria | The owner map remains exact, no former root survives, both Release artifacts are verified, complete unit and applicable gates pass, integration is green or its pre-existing owner-managed failure is explicitly transferred under the execution rule, and the T-level closure audit maps every request to evidence. |
| Original Owner Request | Reorganize NXVM as one VM product: delete `src/vdm`; move `src/vm/app` to `src/app`, `src/core/machine` to `src/core/core`, `src/vm/machine` to `src/core/machine`, `src/vm/profile` to `src/core/profile`; tests follow the same component ownership except independent integration. |
| Similar-Issue Sweep | Scan all tracked production/test/CMake/tool/current-authority paths for former roots, VDM targets, legacy test-owner roots, forwarding includes, and stale owner wording; repair every in-scope hit or stop for a non-layout semantic consumer. |

## Current Technical Baseline

- `vm-0-5-0532` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0532_x64.exe` and `nxvm_0_5_0532_x86.exe` in both
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
| T532 S2 | Accepted implementation `66bba6f3` performs the complete App/Core relocation and test-owner mirror, removes VDM and its consumers, and records 336/336 repository-only unit plus specialized/documentation-gate proof. T532 remains open for its later closure sweep. [Evidence](../etc/evidence/t532-s2-single-product-layout-relocation.md). |
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
