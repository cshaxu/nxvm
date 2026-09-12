# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T528 S2 | Active | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md): remove stale Core-product ownership and move NXVM monitor formatting out of the machine adapter. |

## M5 T528 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved implementation of both queue-head candidates on 2026-09-11 and explicitly approved this S's Core-product removal on 2026-09-11: "banner.h ... src/ root ... core/product ... config.c, config.h ... 挪走". The S1 ledger `833b920c` proved that deleting VM lifecycle ingress now would strand Common requests without an executor; this within-T reorder consumes independently removable ownership first. Incomplete executor completion remains the queued second candidate. |
| Objective | Make `vm/product` the sole owner of NXVM monitor text for machine, BIOS and fault/status reports; delete machine-adapter printing and expose only one copied machine-information query. Move the NXVM banner to `src/` and remove the uncalled `core/product/config` static library so Core contains only `machine`. |
| Non-goals | Do not add Common APIs or implement a Common worker/executor; do not delete `runner`, Core-time pacing, Core execution-context adaptation, profile/media/firmware behavior, or change product lifecycle policy. Do not move the banner into Common/Lib or import SoftPC source. |
| Reference Baseline | `833b920c`; [T528 S1 ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md); [T528 proposal](../proposals/m5-vm-machine-owner-cleanup.md); current Common-machine FIFO/lease contract. |
| Candidate Proposal | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md), S3 product-output portion advanced before the blocked control/lifecycle cutover recorded by S1. |
| Files And ABI Surface | `src/vm/machine/runtime/{machine_info,fault,control,machine_interface}.*`, direct `vm/product/console.*` caller, `src/{banner.h,core/product/{banner.h,config.c,config.h}}`, `src/vm/main.c`, CMake registrations, affected architecture detail and focused owner-local tests. Common/Lib/Core machine interfaces are read-only. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): evidence, full unit and similar-issue sweep. [Architecture](../rules/ARCHITECTURE.md): one state owner and copied cross-owner data boundary. [Coding](../rules/CODING.md): delete retired printing APIs with callers. [Documentation](../rules/DOCUMENT.md): packet/evidence consistency. |
| Verification | Focused product-console/info/fault tests plus source sweeps for `vm_machine_print_*`, `vm_machine_fault_print`, `core_product_parse_memory_kib`, `core-product` and direct `STD_PRINTF` in the affected adapter; full repository-only unit suite; documentation governance; CMake DAG/ownership gates and diff review. |
| Expected Markers | `vm/machine` exports copied machine/fault facts but prints no monitor text; `vm/product` formats all corresponding monitor text; `src/core/` contains only `machine`; Core driver remains opaque and lifecycle/profile/media behavior is unchanged. |
| Asset Needs | None. |
| Reporting Requirements | Record deleted printing APIs and Core-product corpus, retained copied information owner, source/test delta and all gate results in T528 evidence/history. State explicitly that executor cleanup remains T529. |
| Stop Conditions | Stop and report if the copied query would expose a Core pointer or mutable profile/media state, alters lifecycle UX, changes Core timing/profile/media behavior, or discovers a second product consumer needing a different information contract. |
| Exit Criteria | No `vm/machine` monitor-print API or direct print remains in the affected mechanism; `vm/product` produces equivalent machine/BIOS/status/fault text from copied facts; `src/core/` contains only `machine`; eliminated code/callers/target are removed together; full unit and required gates pass. |
| Original Owner Request | "追加2个T任务，第一个清理 vm/machine，第二个将common/machine补全并清理vm/machine" and "准入实施两个T任务". |
| Similar-Issue Sweep | Search every VM/app/product/test/CMake caller of `vm_machine_print_*`, `vm_machine_fault_print`, `vm_machine_fault_get`, `vm_machine_control_print_status`, `core_product_parse_memory_kib`, `core-product` and direct `STD_PRINTF`/`STD_FPRINTF` in the affected machine-adapter files; classify every retained output owner. |

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
| T528 S1 | Ownership ledger accepted: existing Common FIFO/lease is the sole request ingress; incomplete shared executor mechanisms transfer to the queued Common-machine task. [Ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md) |
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
