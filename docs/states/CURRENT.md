# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T528 S7 | Active | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md): complete task-level closure proof and produce the required `0.5.0528` dual-architecture artifacts without changing the established executor boundary. |

## M5 T528 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T528 implementation on 2026-09-11: "准入实施两个T任务"; this is its next continuation after accepted S6 `05408699`. The owner has repeatedly approved normal commits and pushes. |
| Objective | Close T528 only if its complete proposal exit criteria remain proven, with current task identity `0.5.0528` stripped Release artifacts for both Windows architectures. |
| Non-goals | Do not change the established Common-protocol/NXVM-bounded-runner boundary; do not create a generic executor worker, alter Core behavior, profile topology, media, firmware, YAML grammar, public ABI or host synchronization. |
| Reference Baseline | `05408699`; [S1 ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md); [S6 evidence](../etc/evidence/t528-s6-vm-machine-owner-closure-audit.md); the owner-confirmed NXVM versus SoftPC executor distinction. |
| Candidate Proposal | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md), especially S7 and all task exit criteria. |
| Files And ABI Surface | Current-artifact CMake declaration, task/history/status/evidence records, and generated ignored `build/output`/`assets/sessions` artifacts only. No runtime API or behavior change. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): T-level unit/integration/artifact proof and actual-diff review. [Architecture](../rules/ARCHITECTURE.md): one execution owner. [Coding](../rules/CODING.md): no forwarding facade. [Documentation](../rules/DOCUMENT.md): truthful task closure. |
| Verification | Reconfigure/build current Release x64/x86 artifacts; full repository-only unit suite; full owner-managed external integration suite; specialized gates; documentation governance; PE architecture/version/SHA-256 records; actual-diff and final-exit audit. |
| Expected Markers | `nxvm_0_5_0528_x64.exe` and `nxvm_0_5_0528_x86.exe` exist in both mandated ignored directories, are stripped optimized Release PE files of their stated architecture, and all proposal exit criteria map to evidence. |
| Asset Needs | None. Repository-only analysis and tests only. |
| Reporting Requirements | Record commands and outcomes, both artifact identities/architectures/SHA-256 values, the final actual-diff review, and the exit-criterion-to-evidence map. Any new runtime defect requires a revised packet rather than a false closure. |
| Stop Conditions | Stop and revise if full integration exposes a runtime defect, either required architecture cannot build, artifacts do not meet identity/strip/architecture criteria, or final review finds a duplicate owner. |
| Exit Criteria | Every proposal exit criterion is mapped to retained evidence; unit, integration, specialized and documentation gates pass; both artifacts are verified; actual final diff is scoped; the proposal is retained in task history and T528 status is truthfully closed. |
| Original Owner Request | "清理 vm/machine 的重复实现". |
| Similar-Issue Sweep | Re-run the S6 corpus/forwarding and target-owner checks through specialized gates; inspect the final CMake target declaration, all current-artifact references, artifact output paths and task records for stale `0527` identity or a second executor route. |

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
| T528 S5 | Accepted at `2e8c2f7d`: request-to-machine composition is owned by `vm/app`, generic PC/AT firmware mapping by `vm/profile/default_profile`, and Model 40 remains state-coupled composition. [Evidence](../etc/evidence/t528-s5-profile-firmware-owner-disposition.md) records 299/299 unit and specialized-gate proof. |
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
