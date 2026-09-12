# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T528 S6 | Active | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md): exhaustively classify the remaining VM-machine corpus and close only if no duplicate, forwarding or wrong-owner path remains. |

## M5 T528 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T528 implementation on 2026-09-11: "准入实施两个T任务". This final audit consumes the proposal's owner-cleanup exit criteria; it allocates no unproven relocation. |
| Objective | Audit every remaining `src/vm/machine` production file and direct owner edge; either prove its single NXVM adapter responsibility or repair any in-scope forwarding/duplicate/wrong-owner mechanism before task closure. |
| Non-goals | Do not unify NXVM's bounded-quantum Core runner with SoftPC's CCPU/timer runtime; do not alter Core behavior, Common FIFO/lease, profile topology, media, firmware, YAML grammar, public ABI or host synchronization unless a demonstrated duplicate requires a separately revised packet. |
| Reference Baseline | `36d51a29`; [T528 S1 ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md); [S4 evidence](../etc/evidence/t528-s4-provider-lifecycle-cleanup.md); [S5 evidence](../etc/evidence/t528-s5-profile-firmware-owner-disposition.md). |
| Candidate Proposal | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md), task exit criteria and retained owner boundary. |
| Files And ABI Surface | Every tracked `.c`/`.h` below `src/vm/machine`, its CMake source ownership rows and direct project-owner includes. Audit only; an actual new mechanism defect stops this S for a revised repair brief. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): exhaustive exit audit and evidence-based transfer. [Architecture](../rules/ARCHITECTURE.md): one state/route owner and no reverse dependency. [Coding](../rules/CODING.md): no forwarding facade or duplicate abstraction. [Documentation](../rules/DOCUMENT.md): truthful task closure. |
| Verification | Generate a complete file/symbol/owner ledger; sweep forwarding-only and old paths; run dependency DAG and build-owner gates; full repository-only unit suite; specialized gates; documentation governance; actual-diff review. |
| Expected Markers | Every retained file has a distinct NXVM Core adapter, Core-plan composition, copied event/frame adapter or Core-time responsibility. No second request FIFO, lifecycle reducer, presenter/mailbox, media provider, parser, firmware provider or generic executor remains. |
| Asset Needs | None. Repository-only analysis and tests only. |
| Reporting Requirements | Record every file's current owner/reason, exact retained NXVM-vs-SoftPC executor distinction, all searches/gates, code-size delta and any transfer. A clean audit may close T528; any actual defect requires its own revised repair brief. |
| Stop Conditions | Stop and revise before code change if a finding needs Core/Common API changes, a new profile dependency, behavior change, external asset or executor-model change. Do not close on an incomplete inventory. |
| Exit Criteria | The complete residual corpus is classified with evidence; every duplicate category in the proposal is absent or has a distinct documented owner; all gates pass; task-level closure truthfully maps each proposal exit criterion to evidence. |
| Original Owner Request | "清理 vm/machine 的重复实现". |
| Similar-Issue Sweep | Search every retained source/header, CMake target/source row and include edge for mirrored state, direct request queues, lifecycle state, host waits, native presentation, media providers, YAML parsing, firmware mapping, forwarding functions and profile-to-machine reverse dependencies. |

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
