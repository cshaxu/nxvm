# Project Status

## Current Work

## M5 T531 S11 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner directly approved this S on 2026-09-13: re-audit and import the current SoftPC Lib, whose primary change is the `ui-*` to `kvm-*` component rename. One-session coordinator/executor review applies. |
| Objective | Freeze the current committed SoftPC `src/lib` corpus, replace NXVM's complete Lib tree byte-for-byte, and make the direct NXVM consumers use only the canonical KVM component names. |
| Non-goals | Do not write SoftPC; do not import its application, MVDM, assets, firmware, media, tests or dirty worktree files; do not add UI-name aliases or wrappers; do not redesign Common, VM or product policy; do not close T531. |
| Reference Baseline | NXVM `f393637d`; prior Lib source SoftPC `2ea35ce`; candidate source SoftPC committed `987d82e5e87559a00b910a21d2c715d4b844dcae`. The sole dirty SoftPC path is unrelated `assets/binary/softpc.ini` and is excluded. |
| Candidate Proposal | [M5 shared Common product convergence](../proposals/m5-shared-common-product-convergence.md), S11. |
| Files And ABI Surface | Entire `src/lib/` canonical corpus; direct NXVM CMake/Common/VM/test consumers of retired `ui-base`, `ui-window`, and `ui-console` component paths or targets; provenance, evidence, history and current status. Public KVM contracts are exactly those in the frozen source corpus. |
| Applicable Rules | Task Reading Set; Execution lifecycle/actual-change/P review; Architecture single-owner, one-path and dependency-direction rules; Coding public-interface and no-platform-leak rules; Documentation closure rules; source-policy exact-project-owned import/provenance requirements. |
| Verification | Compare imported `src/lib` byte-for-byte to frozen SoftPC tree; run manifest, component-DAG, KVM-name and type-layout checks; sweep retired component paths/targets; build strict x64 and x86 Release; run full repository-only unit suite; run documentation governance and `git diff --check`; rebuild both 0531 stripped artifacts. |
| Expected Markers | `kvm-base`, `kvm-window`, and `kvm-console` are the only KVM component roots/targets in NXVM; no compatibility alias or second Lib path exists; canonical tree comparison exits zero. |
| Asset Needs | None. No YAML or binary asset modification is authorized. |
| Reporting Requirements | Report audit conclusion before import; record frozen commit/revision, changed direct consumers, equality and gate results in evidence; P1 is full implementation and P2 is independent acceptance/governance. |
| Stop Conditions | Stop for a dirty Lib source path, license/provenance change, a canonical source API that cannot be consumed without a local alias/fork, or a build/unit regression not repairable by direct consumer rename adaptation. |
| Exit Criteria | Exact committed canonical Lib imported; every direct caller uses canonical KVM names; old component naming is absent; required gates and x64/x86 artifacts pass; evidence/provenance exist; working tree is clean after accepted P2 while T531 remains open. |
| Original Owner Request | Re-audit and import the current SoftPC Lib; its primary change is renaming `ui-*` components to `kvm-*`. Admit one S task for that work. |
| Similar-Issue Sweep | Sweep all NXVM production, test, CMake and active governance references for retired component directory, include and target spellings; retain historical references only as history. |

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
| T531 S10 | Accepted at `76b9318e`: imported the exact 93-file SoftPC `2ea35ce` Lib corpus without a local fork or consumer shim. Lib boundaries, x64/x86 optimized 0531 builds, 299/299 unit and documentation governance pass. [Closure evidence](../etc/evidence/t531-s10-canonical-lib-refresh.md). |
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
