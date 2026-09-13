# Project Status

## Current Work

## M5 T531 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T531 implementation and directed continued execution after accepted S5. Single-session coordinator/executor review. |
| Objective | Close the finite Common input and asynchronous-failure batch: source retirement, held input, hotkeys, mouse capture, stale facts, queue saturation, and UI/broker/component failure delivery. |
| Non-goals | No SoftPC write, no executor merge, no new product policy callback, no native API outside Lib, no Debug grammar change, no external asset change and no unrelated machine/controller change. |
| Reference Baseline | NXVM `386db334`; read-only SoftPC `e140ec55bbc4cd40956948e57e58dc678e54f699`; T531 proposal and S1 ledger. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md) and [two-product ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md). |
| Files And ABI Surface | `src/common/session/*`, `src/common/ui/*` and their owner tests; NXVM `src/vm/app/*` and `src/vm/product/*` only where a copied Common fact must be bound or a duplicate route removed; only directly required Common manifest/evidence/status records. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution and Documentation Rules; Product UX; T531 input/failure ledger rows. |
| Verification | Frozen input/failure matrix with one disposition per source/lifecycle/presentation context; owner-local stale/queue/full/failure regressions; read-only SoftPC crosswalk; full repository-only unit suite; Common manifest/corpus, documentation and actual-diff gates. |
| Expected Markers | One Session FIFO receives all copied facts; source retirement releases only owned keys; Common UI failure reaches Session without product-side polling or swallowed status; no duplicate input or failure state. |
| Asset Needs | None. No external asset or sibling source import. |
| Reporting Requirements | Record each matrix disposition, product adapter owner, any retained SoftPC receiver and source/test accounting; stop for a required new product-policy seam or a failure that cannot return as a copied fact. |
| Stop Conditions | A required case needs a second queue/state owner, direct UI-to-machine mutation, a generic executor, sibling-repository write, native API leakage or unapproved product policy. |
| Exit Criteria | The complete S6 matrix is reconciled; every NXVM gap is repaired at its Common owner; required verification passes; a complete P is pushed then coordinator-reviewed. |
| Original Owner Request | The two products differ only in startup composition/configuration, injected CLI and injected machine; Common control behavior follows SoftPC while executors remain distinct adapters. |
| Similar-Issue Sweep | Inspect all Common Session/UI facts, input dispatch/hotkey/mouse paths, NXVM UI/result bindings and read-only SoftPC control/input/broker failure paths for duplicate state, silent failure, stale delivery and polling. |

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
