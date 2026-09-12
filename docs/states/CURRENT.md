# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T529 S2 | Active | [M5 Common-machine residual audit and conditional completion](../proposals/m5-common-machine-executor-completion.md): delete the two S1-proven dead VM control paths and close the task without fabricating a Common migration. |

## M5 T529 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved S2 execution and closure on 2026-09-12: "准入清理，然后提交推送收口". |
| Objective | Remove exactly the two S1-proven dead VM control paths, retain every live Core runner/control path, and close T529 with the truthful conclusion that no Common migration was proven. |
| Non-goals | Do not change Common APIs, create an executor worker, change Core/profile/firmware/media/UX behavior, import SoftPC source, or alter active pause/completion semantics. |
| Reference Baseline | `6e7b43a4`; [T529 S1 residual ledger](../etc/evidence/t529-s1-vm-machine-common-residual-ledger.md); [M5 Common-machine residual audit proposal](../proposals/m5-common-machine-executor-completion.md). |
| Candidate Proposal | [M5 Common-machine residual audit and conditional completion](../proposals/m5-common-machine-executor-completion.md), S2--S4 disposition. |
| Files And ABI Surface | `src/vm/machine/runtime/control.[ch]`, their caller/test sweep, task evidence/history/status/proposal/index records, and ignored dual-architecture artifacts. No Common, Core, public ABI, asset or product behavior surface changes. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): actual-diff, full task proof and code-size accounting. [Architecture](../rules/ARCHITECTURE.md): sole owner/no parallel path. [Coding](../rules/CODING.md): remove dead code rather than wrap it. [Documentation](../rules/DOCUMENT.md): truthful closure and transfer. |
| Verification | Exact global caller sweep for both removed names; focused VM control lifecycle smoke; full repository-only unit suite; specialized gates; documentation governance; rebuild/verify current stripped x64/x86 artifacts; retain the previously recorded 39/42 external-integration result without claiming it green. |
| Expected Markers | No production/test/CMake reference to `control_changed` or `vm_machine_control_wait_for_completion`; `completion_ready` and live pause/step paths remain; the S1 ledger's no-Common-migration conclusion remains true. |
| Asset Needs | None. No external source, firmware or media use. |
| Reporting Requirements | Record deleted paths/callers, live-control preservation proof, code-size result, gate outcomes, artifact identities and the owner-approved external-integration exception inherited from the same unresolved boot-chain TODO. |
| Stop Conditions | Stop if either name has a hidden live consumer, removing it changes lifecycle behavior, or a Common API/SoftPC import/new executor is needed. |
| Exit Criteria | Both dead paths are deleted with no residual references; all live control paths remain covered; full unit and specialized gates pass; stripped x64/x86 artifacts are rebuilt; the no-Common-migration disposition, known external-test transfer and actual-diff review are recorded; T529 is truthfully closed. |
| Original Owner Request | "审计所有vm/machine里面剩余的应当属于 common/machine的部分" followed by "准入清理，然后提交推送收口". |
| Similar-Issue Sweep | Re-run the S1 full corpus/caller search for unused `host_sync_event` fields and exported `vm_machine_control_*` functions; verify Common and SoftPC remain untouched and no second executor route appears. |

## Current Technical Baseline

- `vm-0-5-0528` is the current target. Its stripped Release artifacts will be
  `nxvm_0_5_0528_x64.exe` and `nxvm_0_5_0528_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T529 S1 | `6e7b43a4` exhaustively classifies 29 retained VM-machine files against Common and SoftPC, proving no new neutral executor mechanism. The sole receiver is the two-item local-dead-control deletion in S2. [Ledger](../etc/evidence/t529-s1-vm-machine-common-residual-ledger.md). |
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
