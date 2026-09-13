# Project Status

## Current Work

## M5 T531 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continued Common-reuse implementation; execute the independently complete NXVM `console_control` receiver while SoftPC prepares its real Debug adapter. Single-session coordinator/executor review. |
| Objective | Add one copied `console_control` startup value to NXVM session YAML and composition, then have Common Session/UI apply the same Console-display graphical-frame behavior already defined by the SoftPC reference; specify SoftPC's executor-affinity bridge behind the unchanged synchronous Common Debug adapter API. |
| Non-goals | No SoftPC write, no synthetic Debug receiver, no new product-specific presentation callback, no native API in Common, no second Common Debug queue or executor, and no change to raw Console/Window ownership outside the shared reconciliation path. |
| Reference Baseline | NXVM `fae64665`; read-only SoftPC `4c06235`; T531 proposal, S1 two-product ledger and S7 receiver evidence. |
| Candidate Proposal | [Shared Common convergence](../proposals/m5-shared-common-product-convergence.md) and [two-product ledger](../etc/evidence/t531-s1-two-product-convergence-ledger.md). |
| Files And ABI Surface | `src/vm/request_interface.h`, session catalog/parser fixtures, VM composition and Common Session/UI policy contracts/tests; the Common Machine Debug ABI remains unchanged while SoftPC later implements its adapter-side executor bridge. |
| Applicable Rules | Task Reading Set; Architecture, Coding, Execution, Documentation and UX rules; Common remains Lib-only and owns shared control/presentation semantics. |
| Verification | Parser accepts only `0` or `1`; request/config transport is copied; the Common presentation matrix covers Console/Window, text/graphics, running/paused/stopped and both `console_control` values; the later SoftPC adapter proves CCPU-affine execution while preserving the synchronous Common caller contract; focused owner tests plus full repository-only unit suite for code change. |
| Expected Markers | `console_control` has one immutable startup owner, Common derives presentation behavior from it, NXVM no longer invents a divergent display policy, and existing YAML without the field keeps the documented default. |
| Asset Needs | None. Unit fixtures are in source; external assets and output YAML are untouched. |
| Reporting Requirements | Record the SoftPC-reference semantic matrix, every NXVM parser/config/policy route replaced or retained, code-size accounting and full-unit result; report the remaining real SoftPC Debug receiver separately. |
| Stop Conditions | The required common policy cannot express the SoftPC behavior without a new product callback, existing YAML compatibility has no evidence, a platform object leaks into Common, or any scope requires SoftPC source modification. |
| Exit Criteria | Valid/invalid configuration, copied composition, and all presentation-state rows have regression proof; no duplicate NXVM policy route remains; the P is actual-diff reviewed, pushed and accepted with the remaining SoftPC adapter accurately transferred. |
| Original Owner Request | Both products differ only in startup configuration/composition, injected CLI and machine adapter; Common owns control and presentation behavior, and NXVM must also support `console_control`. |
| Similar-Issue Sweep | Inspect every `display` parse/validation/selection route, Common target/reconcile policy, all session YAML fixtures and all raw/cooked Console versus Window transitions for hard-coded product policy. |

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
| T531 S7 | Accepted at `a0446cb5`: Common Debug/XASM32 and NXVM's full paused typed target were audited across the complete operation universe. SoftPC lacks a real receiver; one CCPU safe-point adapter is explicitly transferred without a stub or synthetic result. [Evidence](../etc/evidence/t531-s7-common-debug-receiver-gap.md). |
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
