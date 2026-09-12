# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T528 S5 | Active | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md): relocate request composition to `vm/app` and generic PC/AT firmware to profile ownership; retain only composition coupled to machine state. |

## M5 T528 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T528 implementation on 2026-09-11: "准入实施两个T任务". S5 consumes the already-approved profile/firmware-owner disposition, limited to the two demonstrated non-machine owners below. |
| Objective | Remove session-request composition and generic PC/AT external-ROM provider implementation from `vm/machine`; retain Model 40 composition where moving it would create a profile-to-machine dependency. |
| Non-goals | Do not alter Core execution, bounded-quantum runner, Common FIFO/lease, profile topology, firmware bytes or mappings, YAML grammar, media behavior, Model 40 topology, public ABI or host synchronization. |
| Reference Baseline | `247b67ca`; [T528 S1 ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md); [T528 S4 evidence](../etc/evidence/t528-s4-provider-lifecycle-cleanup.md). |
| Candidate Proposal | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md), S5 profile/firmware-owner disposition. |
| Files And ABI Surface | `src/vm/machine/request_factory.*`, `runtime/rom/external_pc_at.*`, their direct callers/includes and build/verifier paths. `vm/app` receives request composition because it creates machines; `vm/profile/default_profile` receives the generic PC/AT provider. Preserve behavior and avoid a profile-to-machine dependency. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): remove obsolete owner paths and record actual delta. [Architecture](../rules/ARCHITECTURE.md): one owner and no reverse dependency. [Coding](../rules/CODING.md): move whole cohesive responsibilities, not wrappers. [Documentation](../rules/DOCUMENT.md): packet/evidence consistency. |
| Verification | Focused YAML request-resolution, default-PC/AT ROM materialization and Model 40 composition tests; source sweep for old paths and duplicate parser/provider implementations; full repository-only unit suite; specialized gates; documentation governance; actual-diff review. |
| Expected Markers | `vm/app` owns the sole request-to-machine composition; `vm/profile/default_profile` owns the generic PC/AT external-ROM provider; `vm/machine` retains no copy; Model 40 remains a one-way machine composition consumer. |
| Asset Needs | None. Repository-only fixtures only; no firmware or media bytes change. |
| Reporting Requirements | Record old-to-new owner map, caller/path sweep, Model 40 retention proof, code-size delta, and required gates. State the retained distinct NXVM and SoftPC executor boundary. |
| Stop Conditions | Stop if a move changes YAML grammar, firmware mapping/order, exposes machine state, needs a new cross-module API, or proves a second request/parser/provider path. |
| Exit Criteria | Both demonstrated non-machine owners are outside `vm/machine`; old source paths have no references; one parser and one generic PC/AT provider remain; focused/full proof and gates pass; evidence records why coupled Model 40 composition stays. |
| Original Owner Request | "清理 vm/machine 的重复实现". |
| Similar-Issue Sweep | Inspect all `vm/machine` source files, CMake rows and callers for request parsing, profile selection, firmware-provider construction, Model 40 state coupling and forwarding-only wrappers; classify every result as moved, retained with distinct owner, or a separately approved receiver. |

## Current Technical Baseline

- `vm-0-5-0527` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0527_x64.exe` and `nxvm_0_5_0527_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T528 S4 | Accepted at `db79b083`: `machine_devices` is the sole FDD/HDD initialize/reset/finalize owner; the empty provider-lifecycle facade is deleted without changing call order or Core/Common boundaries. [Evidence](../etc/evidence/t528-s4-provider-lifecycle-cleanup.md) records 299/299 unit and specialized-gate proof. |
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
