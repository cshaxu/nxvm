# Project Status

## Current Work

## M5 T531 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continued T531 implementation toward the Common reuse standard. Single-session coordinator/executor review. |
| Objective | Reconcile the finite Common Debug/XASM32 command, paused-target and lifecycle universe against both actual NXVM and read-only SoftPC adapters; implement only a proven NXVM/Common correction and record the exact SoftPC receiver required for real two-consumer Debug. |
| Non-goals | No sibling-repository write, no optional Debug stub, no generic executor, no second CLI loop, no change to original table-driven command grammar, and no product-native API in Common. |
| Reference Baseline | NXVM `d4a1c3c2`; read-only SoftPC `e140ec55bbc4cd40956948e57e58dc678e54f699`; T531 proposal and S1 ledger. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md) and [two-product ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md). |
| Files And ABI Surface | Read-only audit of SoftPC `src/app`/`src/host`; NXVM `src/common/debug/*`, `src/common/xasm32/*`, `src/common/machine/*`, `src/vm/machine/runtime/debug_adapter.*`, owner tests and direct evidence only if an in-scope proven correction exists. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution and Documentation Rules; T531 Debug/XASM32 ledger rows. |
| Verification | Finite operation matrix covering CPU snapshot, memory, port, watch/break, execution plans, lifecycle hand-off, continuation and file commands; direct NXVM owner tests; read-only SoftPC capability crosswalk; full unit suite for any code change. |
| Expected Markers | Common Debug remains the sole parser/state owner; Common Machine remains the sole paused typed-operation boundary; each product adapter has a real receiver or a named receiver gap. |
| Asset Needs | None. No source import, external asset or sibling write. |
| Reporting Requirements | Record every operation's NXVM proof and SoftPC disposition; stop before any SoftPC source change or any new Common operation unsupported by the original command corpus. |
| Stop Conditions | A required SoftPC receiver needs sibling write, a new unsupported Common capability, a raw machine pointer, a second control queue, or a synthetic no-op result. |
| Exit Criteria | The full finite S7 matrix is reconciled, every NXVM/Common gap is repaired if found, and any required SoftPC work is transferred explicitly rather than accepted by mock proof. |
| Original Owner Request | Both products use Common Debug/XASM32; product differences remain injected startup composition, CLI and machine adapter only. |
| Similar-Issue Sweep | Inspect all Common Debug/XASM32 command paths, Common Machine debug requests, NXVM Core adapter operations, and SoftPC app/host machine APIs for duplicated parser state, direct Core access, unsupported operation masking and lifecycle bypass. |

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
| T531 S5 | Accepted at `4b1224b6`: Common Machine now returns actual safe-point request status and owns copied removable-FDD requests; NXVM FDD mutation is routed through stopped/paused adapter safe points and copied completion facts. Start remains Session dispatch because executors do not yet exist. 299/299 unit pass; SoftPC binding remains the explicit later receiver. [Evidence](../etc/evidence/t531-s5-common-machine-protocol.md). |
| T531 S6 | Accepted at `cb697c8c`: Common Session now owns source-local held-key cleanup, run-stamped UI facts and copied UI delivery failure facts; Common UI correctly translates Lib input acceptance and reports Console/broker failure. 299/299 unit pass; SoftPC integration remains the explicit S8 receiver. [Evidence](../etc/evidence/t531-s6-common-input-failure-closure.md). |
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
