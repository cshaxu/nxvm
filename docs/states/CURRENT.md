# Project Status

## Current Work

## M5 T531 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T531 implementation on 2026-09-13. S1 admitted exact canonical-Lib refresh before behavior work; single-session coordinator/executor review. |
| Objective | Replace NXVM `src/lib` byte-for-byte with the reviewed SoftPC `e140ec55bbc4cd40956948e57e58dc678e54f699` corpus and adapt only direct public-contract consumers. |
| Non-goals | No SoftPC app/host import, no Lib-local NXVM patch, no Common behavior redesign, no VM/profile/YAML change and no artifact version change. |
| Reference Baseline | NXVM `1ed83ccb`; SoftPC clean `e140ec55bbc4cd40956948e57e58dc678e54f699`; prior exact NXVM Lib import `3cabea6a`. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md) and [S1 ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md). |
| Files And ABI Surface | Entire `src/lib` corpus and only source/test/CMake consumers of changed public contracts; provenance, evidence and task records. |
| Applicable Rules | Task Reading Set; source policy; Architecture, Coding, Execution and Documentation Rules; T531 ledger canonical-Lib row. |
| Verification | Verify clean source revision and complete tree hash equality; manifest/dependency checks; inspect every changed public contract and all NXVM consumers; complete repository-only unit suite; documentation gate. |
| Expected Markers | One exact Lib corpus, zero stale consumer of changed symbols, no product name or machine semantic in Lib, and every necessary external change listed in evidence. |
| Asset Needs | Project-owner-provided MIT source only. No firmware, media, binary, generated asset or sibling write. |
| Reporting Requirements | Report the exact source revision, file count, consumer adaptations, line accounting, unit/gate outcomes and any ABI blocker before acceptance. |
| Stop Conditions | Source/provenance mismatch; a changed Lib public contract requires a product semantic workaround; external private include; license conflict; integration failure class change. |
| Exit Criteria | Exact imported corpus and manifest, all direct consumers correctly adapted, required unit/gates pass, evidence and P commit pushed; then coordinator actual-diff review. |
| Original Owner Request | Both products share one implementation and SoftPC control experience; S1 requires exact canonical Lib before Common behavior work. |
| Similar-Issue Sweep | Compare the full Lib tree, then search all tracked NXVM production/test/CMake uses of every changed public header/symbol; classify each migrated, deleted or not applicable. |

## Current Technical Baseline

- `vm-0-5-0530` is the current target. Its recorded stripped Release artifacts are
  `nxvm_0_5_0530_x64.exe` and `nxvm_0_5_0530_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T531 S1 | Accepted at `3136645c`: the finite 14-row NXVM/SoftPC/Common ledger freezes the `e140ec5` SoftPC reference, classifies every control, UI, input, machine, Debug and XASM32 path, and inserts exact canonical-Lib refresh as S2. No runtime code changed. [Ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md). |
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
