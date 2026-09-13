# Project Status

## Current Work

## M5 T531 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T531 implementation on 2026-09-13; S1 ledger and accepted S2 identify the Common UI completion model as the next bounded receiver. Single-session coordinator/executor review. |
| Objective | Give Common UI one explicit action/completion protocol for Window, raw VM Console and monitor Console binding, without selecting presentation policy. |
| Non-goals | No SoftPC repository write, no CLI grammar, lifecycle reducer, console_control parse, machine executor change, Debug change or product-specific display rule. |
| Reference Baseline | NXVM `566903e6`; exact canonical Lib SoftPC `e140ec55bbc4cd40956948e57e58dc678e54f699`; T531 S1 ledger. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md) and [two-product ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md). |
| Files And ABI Surface | `src/common/ui/*`, its public interface and owner-local tests; NXVM callers only where needed to preserve build while S4 remains the sole policy/reducer migration receiver. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution and Documentation Rules; Product UX; T531 UI/topology/frame/title/mouse/failure ledger rows. |
| Verification | Owner-local UI unit tests for every create/destroy/bind completion/failure and frame-after-recreate path; complete repository-only unit suite; Common manifest/corpus and documentation gates; actual-diff review. |
| Expected Markers | One action vocabulary; copied actual facts; no action assumed complete before its completion; Console broker replacement returns failure; every leaf uses one Common UI owner. |
| Asset Needs | None. No external asset or sibling source import. |
| Reporting Requirements | Record action/fact contract, retained temporary caller boundary, source/test line accounting and all failure-path results. |
| Stop Conditions | A required effect needs product policy, native handle leakage, a second UI loop, Lib public ABI change, or any consumer cannot migrate without an unapproved lifecycle change. |
| Exit Criteria | Common UI exposes and internally uses the explicit protocol; focused and full unit/gates pass; any transitional caller is finite, documented and assigned to S4; complete P pushed then coordinator-reviewed. |
| Original Owner Request | Both products share SoftPC control experience and only vary in composition/startup config, injected CLI and injected machine. Common, not a product policy callback, owns shared UI mechanics. |
| Similar-Issue Sweep | Inspect all Common UI, NXVM UI callers and SoftPC presentation action forms for target/action, component existence, broker binding, frame delivery, title, mouse and failure handling. |


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
| T531 S2 | Accepted at `d9bf9338`: exact 93-file SoftPC `e140ec5` Lib replacement; 93/93 blob match, manifest/dependency checks and 299/299 repository-only units. No product route changed. [Evidence](../etc/evidence/t531-s2-canonical-lib-refresh.md). |
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
