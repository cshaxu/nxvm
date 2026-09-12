# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T529 S1 | Active | [M5 Common-machine residual audit and conditional completion](../proposals/m5-common-machine-executor-completion.md): exhaustively classify all remaining `vm/machine` mechanisms against Common and SoftPC before any migration. |

## M5 T529 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved admission on 2026-09-12: "准入下一个T任务，审计所有vm/machine里面剩余的应当属于 common/machine的部分". |
| Objective | Produce the finite two-consumer residual ledger for every remaining `src/vm/machine` production responsibility and identify only real Common duplicates or real shared candidates. |
| Non-goals | Do not move code, modify Common APIs, create an executor worker, change Core/profile/firmware/media/UX behavior, or import SoftPC source in S1. Do not assume the NXVM bounded runner and SoftPC CCPU/timer loop are equivalent. |
| Reference Baseline | `cab545e8`; [T528 S6 closure audit](../etc/evidence/t528-s6-vm-machine-owner-closure-audit.md); [M5 Common-machine residual audit proposal](../proposals/m5-common-machine-executor-completion.md); read-only `O:/repos.hobby/softpc` comparison corpus. |
| Candidate Proposal | [M5 Common-machine residual audit and conditional completion](../proposals/m5-common-machine-executor-completion.md), S1. |
| Files And ABI Surface | Documentation/evidence only: `docs/etc/evidence/t529-s1-*`, task status/proposal/index records. No source, build, ABI, asset or product behavior change. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): coverage ledger and truthful dispositions. [Architecture](../rules/ARCHITECTURE.md): one owner and neutral dependency direction. [Coding](../rules/CODING.md): no forwarding facade or speculative abstraction. [Documentation](../rules/DOCUMENT.md): active-packet/evidence topology. |
| Verification | Exhaustive `vm/machine` file/symbol/field/caller inventory; direct source comparison against current Common and read-only SoftPC routes; neutral-term/dependency scan; documentation governance. |
| Expected Markers | A durable ledger maps every residual mechanism to existing Common duplicate, demonstrated two-consumer candidate, distinct NXVM adapter, or named receiver, with concrete source evidence and no unclassified rows. |
| Asset Needs | Read-only source comparison with `O:/repos.hobby/softpc`; no source import, firmware, media or external binary. |
| Reporting Requirements | Record the frozen universe, per-row owner/disposition, Common and SoftPC proof paths, excluded worker semantics, and any later S2 batch. Report a material architecture contradiction before any implementation. |
| Stop Conditions | Stop before implementation if the evidence requires a Common API/product behavior change, a SoftPC source import, a new executor owner, or cannot distinguish a purported duplicate from a product adapter. |
| Exit Criteria | The complete finite universe is classified and evidence-indexed; all claimed Common candidates have two real consumers; all unshared NXVM paths have a distinct reason or receiver; documentation governance passes; no code changes occur. |
| Original Owner Request | "审计所有vm/machine里面剩余的应当属于 common/machine的部分 (比如，已经在 common/machine的重复实现，或者softpc也有的且应当被共用的)". |
| Similar-Issue Sweep | Search every `src/vm/machine` C/header/CMake source row plus `src/common/machine` and SoftPC `src/app`, `src/host/machine` for queue, run, wake, safe-point, lifecycle, completion, wait, debug-lease, frame and adapter responsibilities. |

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
