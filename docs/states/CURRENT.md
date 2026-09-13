# Project Status

## Current Work

## M5 T531 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T531 implementation; accepted S1-S3 establish the finite Common session/control gap. Single-session coordinator/executor review. |
| Objective | Make Common session the one FIFO/reducer and monitor scheduler for requested, in-flight and acknowledged machine and UI facts, using the completed Common UI action protocol. |
| Non-goals | No SoftPC write, no generic executor, no Core/CCPU execution-loop merge, no Debug behavior change, no external asset change and no product-specific display-policy callback. |
| Reference Baseline | NXVM `9550da4b`; SoftPC `e140ec55bbc4cd40956948e57e58dc678e54f699`; T531 S1 ledger; accepted S3 action/fact evidence. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md), [two-product ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md), [S3 evidence](../etc/evidence/t531-s3-common-ui-completion.md) and [S4 evidence](../etc/evidence/t531-s4-common-session-control.md). |
| Files And ABI Surface | `src/common/session/*`, Common UI/session owner tests, and NXVM integration only where the old direct control loop must submit or consume the Common protocol. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution and Documentation Rules; Product UX; T531 monitor/lifecycle/UI topology rows. |
| Verification | Owner-local requested/in-flight/acknowledged transition and failure tests; monitor line, prompt and lifecycle-notice tests; full repository-only unit suite; Common manifest/corpus and documentation gates; actual-diff review. |
| Expected Markers | One session FIFO; actions remain pending until completion facts; Common session owns monitor scheduling and lifecycle notices; product seams remain composition/config, injected CLI and injected machine only. |
| Asset Needs | None. No external asset or sibling source import. |
| Reporting Requirements | Record the state model, retained executor boundary, exact removed target-plan route, source/test accounting and transition/failure results. |
| Stop Conditions | A needed operation requires a second product control loop, native API/handle, generic executor, unapproved product policy, or cannot preserve the existing CLI injection seam. |
| Exit Criteria | Common session consumes the full S4 ledger batch through one reducer; replaced target-plan control is removed; required tests/gates pass; complete P is pushed then coordinator-reviewed. |
| Original Owner Request | Shared product experience follows SoftPC; product variation is only startup composition/config, injected CLI and injected machine. Common session coordinates selected raw Console, Window and cooked monitor transitions. |
| Similar-Issue Sweep | Inspect all Common session facts/reducers, Common UI actions, NXVM console/session callers and SoftPC control/reconciler transitions for duplicate requested/current/in-flight state, monitor ownership and direct lifecycle notices. |

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
| T531 S3 | Accepted at `4fd3d29f`: one Common UI action/completion owner reports copied Window/raw-Console/broker facts; injected failures and 299/299 repository-only units pass. Its finite legacy target-plan translation is assigned to S4. [Evidence](../etc/evidence/t531-s3-common-ui-completion.md). |
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
