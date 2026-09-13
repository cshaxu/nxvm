# Project Status

## Current Work

## M5 T531 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T531 implementation and directed continued execution after accepted S4. Single-session coordinator/executor review. |
| Objective | Complete the Common machine request/result boundary needed by both existing adapters for start, lifecycle, host input and safe-point media operations, while retaining their distinct Core and CCPU executors. |
| Non-goals | No SoftPC write, no generic executor worker, no Core/CCPU merge, no direct controller/media mutation from Session or UI, no Debug grammar change and no external asset change. |
| Reference Baseline | NXVM `947fed3e`; read-only SoftPC `e140ec55bbc4cd40956948e57e58dc678e54f699`; T531 proposal and S1 ledger. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md) and [two-product ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md). |
| Files And ABI Surface | `src/common/machine/*` and `src/common/session/*` copied contracts, NXVM `src/vm/machine/*` and `src/vm/app/*` adapter callers, `src/vm/product/console.c` command cutover, owner-local machine/session tests and only directly required Common documentation/manifest records. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution and Documentation Rules; Product UX; T531 lifecycle, executor and product-seam ledger rows. |
| Verification | Complete request-kind and stale-generation matrix; NXVM Core safe-point start/lifecycle/input/media regressions; explicit read-only SoftPC adapter crosswalk; full repository-only unit suite; Common manifest/corpus, documentation and actual-diff gates. |
| Expected Markers | One Common copied request FIFO and run generation; machine adapters consume requests only at their own safe point; no second worker or UI-to-machine bypass; media completion is explicit rather than inferred. |
| Asset Needs | None. No external asset or sibling source import. |
| Reporting Requirements | Record every request/result disposition, actual adapter owner, retained executor distinction, source/test accounting and any SoftPC integration receiver that cannot be proved in this repository. |
| Stop Conditions | A required request cannot be represented without a product command envelope, a common executor, a direct Core/CCPU mutation bypass, a sibling-repository write, or a new unapproved product-policy seam. |
| Exit Criteria | The full S5 ledger batch has one disposition per request/result; NXVM uses the completed Common protocol with obsolete duplicate transport removed; all required verification passes; complete P is pushed then coordinator-reviewed. |
| Original Owner Request | The two products differ only in startup composition/configuration, injected CLI and injected machine; common control behavior follows SoftPC while executors remain distinct adapters. |
| Similar-Issue Sweep | Inspect all Common-machine queue/generation/lease paths, NXVM runner/control/media callers, and read-only SoftPC runtime/input/media safe points for duplicate transport state, direct mutations, stale generations and hidden synchronous completion. |

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
| T531 S4 | Accepted at `8718c40b`: Session now owns requested/in-flight/acknowledged surface control and monitor endpoint; the legacy target-plan route is deleted. Ordered Window/raw-Console/monitor transitions, failure/reconcile retry, strict source compile and 299/299 unit pass. [Evidence](../etc/evidence/t531-s4-common-session-control.md). |
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
