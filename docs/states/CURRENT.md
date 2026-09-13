# Project Status

## Current Work

## M5 T531 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved on 2026-09-13: close T530, admit the queue-first Common convergence task; both products share SoftPC control UX, console_control, Debug and XASM32. Single-session coordinator/executor review. |
| Objective | Freeze a complete two-product capability and transition ledger, classify actual Common/SoftPC gaps and approve bounded implementation batches before runtime edits. |
| Non-goals | No runtime implementation in S1; no second policy seam, generic worker, source fork, sibling modification or change to owner YAML. |
| Reference Baseline | NXVM bc9a1247; T530 pinned Lib 3cabea6a; record the exact current SoftPC app/host and Lib revisions during inventory. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md). |
| Files And ABI Surface | Common public interfaces and implementation; NXVM VM bindings; read-only SoftPC app/host/Lib; task proposal and forthcoming indexed transition ledger. |
| Applicable Rules | Task Reading Set, Execution, Documentation, Architecture and Coding authorities, Product UX and source policy; proposal records approved future design, not completed current capability. |
| Verification | Read actual code for every ledger family; trace both callers and owners; reconcile finite coverage, proposed removals and adapter gaps; documentation governance and actual-diff coordinator review. S1 makes no runtime change. |
| Expected Markers | Every required family has both source locations, ownership, gap disposition, implementation batch and regression owner; only three product variability seams. |
| Asset Needs | No new external asset, source import or artifact version change during this planning batch. |
| Reporting Requirements | Report full gap inventory, execution order, blockers and exact source revisions; never call two-product support complete from mocks. |
| Stop Conditions | Missing real adapter capability or inaccessible reference; incompatible source terms; required extra product policy; sibling integration lacks an approved execution boundary. |
| Exit Criteria | Complete reviewed two-consumer ledger and bounded S plan, governance checks pass and planning delivery committed/pushed. Runtime work begins only under the next admitted packet. |
| Original Owner Request | Close the current T; queue and admit shared Common convergence. Product differences only startup configuration/composition, injected CLI and injected machine; both include console_control, Debug and XASM32, using SoftPC control UX. |
| Similar-Issue Sweep | Inventory all lifecycle, presentation, monitor, input, media and Debug owners in both products; classify duplicate paths and existing shared implementations before proposing additions. |

## Current Technical Baseline

- `vm-0-5-0530` is the current target. Its recorded stripped Release artifacts are
  `nxvm_0_5_0530_x64.exe` and `nxvm_0_5_0530_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
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
