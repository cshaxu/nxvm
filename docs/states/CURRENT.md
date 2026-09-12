# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T528 S4 | Active | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md): remove the empty VM provider-lifecycle forwarding layer without changing executor or profile behavior. |

## M5 T528 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T528 implementation on 2026-09-11: "准入实施两个T任务". T528's retained S4 boundary explicitly permits folding provider lifecycle into the owner-local media/profile composition transaction. This packet consumes only the demonstrated zero-state forwarding layer. |
| Objective | Delete `vm/machine/runtime/provider_lifecycle.*`; have the existing device-composition owner directly initialize, reset and finalize its FDD/HDD providers. |
| Non-goals | Do not change Core run/quantum execution, Common FIFO/lease, profile topology, ROM mapping, media modes, provider order, Core media ABI, Model 40 composition or host synchronization. |
| Reference Baseline | `ac554fb0`; [T528 S1 ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md); [T528 S3 media relocation evidence](../etc/evidence/t528-s3-media-owner-relocation.md). |
| Candidate Proposal | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md), S4 owner-local composition cleanup. |
| Files And ABI Surface | `src/vm/machine/runtime/provider_lifecycle.*`, its sole `control.c` caller, device-composition headers and focused lifecycle/media tests. No public API changes. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): remove forwarding/obsolete paths and record actual delta. [Architecture](../rules/ARCHITECTURE.md): one mutable provider lifecycle owner. [Coding](../rules/CODING.md): no wrapper with no semantic responsibility. [Documentation](../rules/DOCUMENT.md): packet/evidence consistency. |
| Verification | Focused machine-media lifecycle and initialization-atomicity tests; source sweep for provider-lifecycle symbols and duplicate FDD/HDD initialize/reset/finalize calls; full repository-only unit suite; specialized gates; documentation governance; actual-diff review. |
| Expected Markers | `machine_devices` is the sole FDD/HDD provider lifecycle owner; no `provider_lifecycle` source, symbol or forwarding call remains. |
| Asset Needs | None. Repository-only test fixtures only. |
| Reporting Requirements | Record old-to-new call ownership, lifecycle-order proof, source sweep, code-size delta and required gates. State that executor and profile/firmware relocation remain separate. |
| Stop Conditions | Stop if deleting the facade changes initialization/reset/finalize order, requires Core/Common/Lib API changes, exposes machine state, or identifies a second provider lifecycle owner. |
| Exit Criteria | The forwarding source/header are deleted; all former callers use the real device owner; one FDD/HDD provider lifecycle route remains; required tests/gates pass; evidence documents the retained executor/profile boundary. |
| Original Owner Request | "清理 vm/machine 的重复实现". |
| Similar-Issue Sweep | Search all production/test/CMake references to provider lifecycle, device initialization/reset/finalize and FDD/HDD provider binding; classify each as device owner, Core provider registry, profile policy or test. |

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
| T528 S3 | Accepted at `1a1e9e90`: `vm/media` is the sole FDD/HDD geometry, Lib-storage and Core-provider owner; device composition has the one registry bind/freeze route, and no reverse dependency remains. [Evidence](../etc/evidence/t528-s3-media-owner-relocation.md) records 299/299 unit and specialized-gate proof. |
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
