# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T528 S3 | Active | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md): relocate NXVM FDD/HDD storage-to-Core media adaptation to `vm/media`, without executor or lifecycle changes. |

## M5 T528 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T528 implementation on 2026-09-11: "准入实施两个T任务". S1 classified FDD/HDD as NXVM-local media adapters; S2 completed independent product/Core cleanup. This reordered next S remains within the approved T528 owner-cleanup boundary and does not consume the unresolved executor claim. |
| Objective | Make `vm/media` the sole NXVM owner for FDD/HDD geometry, Lib storage medium and Core-media-provider adaptation; remove those generic media implementation files from `vm/machine`. |
| Non-goals | Do not change media formats, image policy, read-only/direct/overlay semantics, Core media provider ABI, profile topology, Model-40 composition, firmware/ROM mapping, lifecycle/executor control or Common APIs. Do not move tests outside `test/vm/`. |
| Reference Baseline | `67902bf6`; [T528 S1 ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md); [T528 proposal](../proposals/m5-vm-machine-owner-cleanup.md); Lib storage interface. |
| Candidate Proposal | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md), reordered S3 media-owner batch. |
| Files And ABI Surface | `src/vm/machine/{fdd,hdd}{,_private}.h/.c`, `src/vm/machine/runtime/media.*`, their private VM callers and CMake paths; `test/vm/machine/*media*` plus affected Core-media/profile tests. Public symbols retain `vm_machine_*` names only where Core-machine integration consumes them; no new public capability. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): structural relocation and code-size accounting. [Architecture](../rules/ARCHITECTURE.md): one media-state owner, one Core provider route. [Coding](../rules/CODING.md): use `git mv`, remove old include paths together. [Documentation](../rules/DOCUMENT.md): packet/evidence consistency. |
| Verification | Focused FDD/HDD storage/provider/media lifecycle tests; source sweep for old paths and duplicate Core media provider definitions; full repository-only unit suite; specialized gates; documentation governance; actual-diff review. |
| Expected Markers | `src/vm/media/` owns each FDD/HDD medium and Core provider adapter; `vm/machine` contains no FDD/HDD implementation or storage medium owner; all Core provider bindings retain exactly one producer route. |
| Asset Needs | None. Repository-only test fixtures only. |
| Reporting Requirements | Record every moved file/caller, retained symbol name, media-provider uniqueness sweep, code-size delta and gate results. State that executor disposition remains separate. |
| Stop Conditions | Stop and report if relocation requires a Core/Lib/Common API change, alters a profile's media topology, exposes media state across a public boundary, or finds a second production media owner. |
| Exit Criteria | All scoped media implementation files are under `vm/media`; no old include/source path or duplicate implementation remains; Core receives the same sole providers; all required tests/gates pass; current S evidence names retained adapter vs transferred executor boundary. |
| Original Owner Request | "清理 vm/machine 的重复实现" and "vm/machine里面，哪些是和 common/machine 或者其他common和lib组件重复的部分应当清除？" |
| Similar-Issue Sweep | Search all production/test/CMake references to FDD/HDD media providers, `lib_storage_medium`, media mount/eject and image-geometry helpers; classify every result as `vm/media`, Core, Lib storage or profile policy. |

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
| T528 S2 | Accepted at `27b27298`: `vm/product` is the only monitor-text owner; `vm/machine` exposes copied information only; dead Core-product config is deleted and Core contains only `machine`. [Evidence](../etc/evidence/t528-s2-product-boundary-cleanup.md) transfers the executor receiver to T529. |
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
