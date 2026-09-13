# Project Status

## Current Work

## M5 T531 S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved this 2026-09-13 the next T531 subtask: import the current SoftPC Lib. One-session coordinator/executor review. |
| Objective | Import the complete current project-owned SoftPC `src/lib/` corpus into NXVM byte-for-byte, then make only source-proven NXVM consumer adaptations required to compile and use that canonical Lib. |
| Non-goals | No SoftPC write; no import of SoftPC app/MVDM, tests, assets, firmware, media or binaries; no NXVM-local Lib patch; no Common/VM redesign beyond direct compile-required adaptation; no T531 closure. |
| Reference Baseline | NXVM `b7783386`; SoftPC `2ea35ce87bb538a6cc520be1353641a99a94d427`; source path `src/lib`; SoftPC's sole dirty file is outside the corpus. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md), S10. |
| Files And ABI Surface | Entire `src/lib/` 93-file corpus, its manifest/build metadata, and only direct NXVM Common/VM consumers, tests, CMake/provenance/evidence/status needed by audited API deltas. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution and Documentation rules; source policy; Lib remains product-neutral and original SoftPC source is copied unchanged. |
| Verification | Compare every tracked Lib path and SHA-256 with the frozen SoftPC revision; verify Lib manifest/component dependencies; build x64/x86; run focused affected tests and full repository-only unit suite; inspect all consumer changes and run documentation governance. |
| Expected Markers | Exactly one 93-file NXVM Lib corpus matches SoftPC; no product terminology enters Lib; any consumer adaptation is outside Lib and has one retained route; no ignored SoftPC working-tree input is imported. |
| Asset Needs | Build the existing stripped optimized 0531 x64/x86 executables in `build/output` and `assets/sessions`; do not edit YAML or external assets. |
| Reporting Requirements | Record source revision, exact file/hash comparison, imported versus adapted path inventory, ABI delta dispositions, code-size accounting, verification and artifact hashes. |
| Stop Conditions | SoftPC Lib is dirty, contains independent third-party material, needs a product/native API in Lib, has an API delta requiring a parallel NXVM path, or a consumer cannot adapt without a separate approved design change. |
| Exit Criteria | Every 93 Lib path matches the frozen SoftPC corpus; provenance/index/evidence are complete; all needed NXVM consumers use the imported API with no parallel Lib route; required x64/x86 builds, full unit and governance gates pass; reviewed implementation is committed and pushed. T531 remains open. |
| Original Owner Request | Add an S task to import the latest SoftPC Lib. |
| Similar-Issue Sweep | Compare all Lib paths, CMake source lists, manifests, public headers and every NXVM include/call site of changed Lib APIs; inspect the entire SoftPC status so only the frozen Lib corpus is imported. |

## Current Technical Baseline

- `vm-0-5-0531` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0531_x64.exe` and `nxvm_0_5_0531_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T531 S9 | Accepted at `7e9a9b01`: removed three no-consumer Common APIs and the unconfigured presentation fallback; documented borrowed binding/reducer ownership; corrected the finite XASM32 diagnostics and TR dispatch rows while retaining its table style. Common manifest/x64/x86 builds, 299/299 unit, documentation governance and optimized 0531 x64/x86 artifact proof pass. [Closure evidence](../etc/evidence/t531-s9-common-api-xasm32-cleanup.md). |
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
