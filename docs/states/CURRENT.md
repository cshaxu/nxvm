# Project Status

## Current Work

## M5 T531 S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved this 2026-09-13 Common audit cleanup after reviewing the findings. One-session coordinator/executor review. |
| Objective | Remove the audit-proven unused Common UI exports and Session target fallback, make the remaining borrowed-binding and reducer-thread contracts explicit, and eliminate all current XASM32 compiler warnings while retaining its original table-driven behavior. |
| Non-goals | No SoftPC write, no new Common API, no product/native API, no new executor or queue, no Debug grammar/output/semantic redesign, and no T531 closure. |
| Reference Baseline | `2348ab2f`; T531 S8 closure evidence; current NXVM and read-only SoftPC consumer-call audit. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md). |
| Files And ABI Surface | `src/common/{ui,session,xasm32}` interfaces/implementations, their Common tests and manifest; evidence/index/current progress at closure. Public UI/session symbols may be removed only after the complete NXVM and read-only SoftPC call sweep. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution and Documentation rules; Common remains Lib-only and product-neutral; original XASM32 table structure is a preservation constraint. |
| Verification | Repeat the complete NXVM and read-only SoftPC consumer sweep; Common standalone corpus/manifest checks; clean x64 and x86 Common builds with zero XASM32 diagnostics; focused Common tests and the full repository-only unit suite. |
| Expected Markers | No caller or declaration remains for the removed UI/session APIs; only one presentation-policy route remains; public interfaces state ownership/thread/lifetime facts; XASM32 builds warning-free without changing command/parser conformance. |
| Asset Needs | Build the stripped optimized `0531` x64/x86 executables in `build/output` and `assets/sessions`; do not modify external assets or YAML. |
| Reporting Requirements | Record the removed API/path inventory, retained owner contracts, exact compiler warning before/after result, code-size accounting, focused/full verification and any behavior-preservation comparison. |
| Stop Conditions | A checked real consumer needs a removed API, warning removal requires a semantic/table-flow change, or the required lifecycle contract cannot be expressed without exposing product/platform state. |
| Exit Criteria | Every audit finding has an implemented or evidence-backed non-applicable disposition; Common remains manifest-valid and Lib-only; x64/x86 XASM32 diagnostics are zero; focused/full units pass; actual diff is reviewed, committed and pushed. T531 remains open for its real SoftPC adapter evidence. |
| Original Owner Request | Repair every Common audit finding, including XASM32. |
| Similar-Issue Sweep | Inspect every Common public interface for consumer reachability, every public borrowed callback/pointer for lifetime/thread ownership, every unconfigured presentation branch, and every XASM32 diagnostic on both supported architectures. |

## Current Technical Baseline

- `vm-0-5-0531` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0531_x64.exe` and `nxvm_0_5_0531_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T531 S8 | Accepted at `P8`: NXVM carries copied `console_control` into the Common-owned presentation policy; Common Machine has one optional, post-copy empty-to-nonempty readiness wake for either product's existing executor; and the synchronous paused-Debug adapter contract remains fixed without a second queue or worker. The standalone Common corpus builds on x64 and x86, and the repository-only unit suite is 299/299. [Closure evidence](../etc/evidence/t531-s8-closure.md). |
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
