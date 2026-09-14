# Project Status

## Current Work

## M5 T531 S12 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner authorized the committed SoftPC corpus import on 2026-09-14. The source is frozen by commit, not by later uncommitted SoftPC worktree content; complete this S but keep T531 open for owner direction. |
| Objective | Replace NXVM's complete Lib, Common and their standalone test corpora with the exact frozen SoftPC corpus, then directly adapt NXVM consumers to its public contracts without a local fork or compatibility layer. |
| Non-goals | Do not write SoftPC; do not import app, host, MVDM, assets, firmware, media or product tests; do not close T531; do not claim SoftPC runtime integration or whole-task two-product execution acceptance; do not edit output YAML. |
| Reference Baseline | NXVM `e894ef89`; SoftPC committed shared-corpus source `662ed4b390b886800c202a66eac754d9aac595b6`, consisting of 94 Lib, 36 Common, 38 Lib-test and 14 Common-test files. Uncommitted SoftPC worktree changes are excluded. |
| Candidate Proposal | [M5 shared Common product convergence](../proposals/m5-shared-common-product-convergence.md), revised S12. |
| Files And ABI Surface | Exact `src/lib/`, `src/common/`, `test/lib/` and `test/common/` corpus import; NXVM CMake, VM/App/Common consumer adaptations, test aggregation, provenance, evidence, history and current status. Public source contracts are exclusively the frozen SoftPC interfaces. |
| Applicable Rules | Task Reading Set; Execution lifecycle and actual-change review; architecture single-owner/one-path/dependency-direction rules; coding public-interface/no-platform-leak rules; documentation closure rules; source-policy project-owned exact-import/provenance requirements. |
| Verification | Archive the named SoftPC commit; compare each of the four imported trees byte-for-byte; run Lib and Common manifest/DAG/corpus/type/KVM gates; sweep NXVM for retired and duplicate consumer paths; build strict x64/x86 Release; run the full repository-only unit suite; run documentation governance and `git diff --check`; rebuild both 0531 stripped artifacts. |
| Expected Markers | All four NXVM imported trees exactly equal SoftPC `662ed4b`; Common contains no platform bypass; NXVM uses the imported public contracts directly and retains no compatibility shim or parallel Common control/UI/machine path. |
| Asset Needs | None. No YAML or binary asset modification is authorized. |
| Reporting Requirements | Record frozen source commit, provenance, tree counts/equality, removed or adapted NXVM consumer paths, line accounting and every gate in evidence; P1 is the full implementation delivery and P2 is independent acceptance/governance. |
| Stop Conditions | Stop for a dirty source corpus, license/provenance change, an unconsumable public-contract conflict requiring a local fork/shim, residual product/platform leakage, or a build/unit regression not repairable by direct NXVM consumer adaptation. |
| Exit Criteria | Exact committed four-tree corpus imported; no local compatibility path remains; required source and NXVM gates plus x64/x86 artifacts pass; evidence/provenance exist; P1 and independent P2 are pushed; working tree is clean while T531 remains open for S13. |
| Original Owner Request | Import the committed SoftPC corpus, complete modification, build, test, commit, push and S closure, then await the next instruction without closing T531. |
| Similar-Issue Sweep | Sweep tracked NXVM Lib/Common/Test/CMake/VM/App consumers for direct platform code in Common, retired corpus names, local fork files, aliases and duplicate control/UI/machine implementations; fix every in-scope production/test/build hit or explicitly record its distinct remaining owner. |

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
