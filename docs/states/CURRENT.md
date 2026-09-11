# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Active | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): create a neutral common runtime mechanism below `lib` and converge NXVM onto its one Core/product/app route. |

## M5 T527 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved admission in this task on 2026-09-11. Scope is S1 only: route/portability ledger and immutable common-contract design. No implementation, import, Core/VM move, ABI mutation, asset mutation, artifact build or task closure is approved in S1. |
| Objective | Produce the finite actual-route and portability ledger required to implement the approved end state: independent `core/{machine,product,debug}`, neutral `common/{contracts.h,ui,session,machine}`, and NXVM `vm/{machine,product,app}`. Freeze one-owner queue, run-generation, adapter and deletion contracts before code changes. |
| Non-goals | Do not create `common/runtime`; move source; change existing production behavior; import/copy/modify SoftPC; change `src/lib`; change Core semantics, timing, firmware, profiles, assets, tests, build targets or artifacts; reserve later S acceptance; or claim SoftPC adoption. |
| Reference Baseline | NXVM `98ac51a4` (`M5 T526 S13 P2`) with canonical `src/lib` manifest SHA-256 `47E2DAA9A47F9FAD6DB680399DCE509E42C04A3EB4BCFEB61C4BFCBE875E1400`; read-only SoftPC `291afe4` is comparison evidence only. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), S1 actual-route and portability ledger. |
| Files And ABI Surface | S1 may add only `docs/etc/evidence/t527-s1-common-runtime-route-ledger.md` and update task-history/status records required by its accepted P. It may inspect but must not change `src/core`, `src/vm`, `src/lib`, `test`, CMake, assets, SoftPC or any public ABI. Ledger must name the proposed copied `common` contracts, but no header is created in this S. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): active-packet, S/P lifecycle, actual-change review, evidence and similar-issue sweep. [Architecture](../rules/ARCHITECTURE.md): neutral dependency direction, one owner/route, opaque copied cross-module interfaces and composition-root-only integration. [Coding](../rules/CODING.md): no speculative framework, no forwarding wrappers, remove replacement paths in their owning S. [Documentation](../rules/DOCUMENT.md): concise, non-competing evidence/status. [Source policy](../etc/operations/policy/source-policy.md): SoftPC is read-only evidence; no source import/derivation. |
| Verification | Review every current NXVM route in `core`, `vm` and `src/lib` named by the proposal, and relevant read-only SoftPC `app`/`host` routes. The ledger must map each state, queue, producer, consumer, thread boundary, opaque handle, public operation, product-only field and old-path deletion receiver. Run complete repository unit suite and documentation governance; run `git diff --check`; verify S1 changes are documentation/evidence only. No integration or artifact build is required by this design-only S; they remain mandatory for affected later implementation S and T closure. |
| Expected Markers | One finite evidence ledger with: component dependency graph; queue table; run-id/stale-event rules; copied API vocabulary; Core-debug lifecycle/target contract; NXVM-only versus SoftPC-only disposition; exact legacy-path deletion receiver; forbidden-vocabulary sweep; and explicit conclusion that no `common/runtime` is needed. |
| Asset Needs | None. No firmware, media, font, YAML, external binary, trace or third-party source is an S1 input or output. |
| Reporting Requirements | Executor first confirms the scope or records a material objection. Report one compact progress node after the ledger has a complete route inventory. Deliver one reviewable, pushed P only after every S1 ledger row, full unit, documentation governance, diff check and self-review pass. Coordinator then independently reviews the actual documentation change before accepting S1; do not admit S2 without owner approval. |
| Stop Conditions | Stop and report before changing source if any proposed common contract requires a product/Core/MVDM/native type, a raw pointer, a second owner/route, an unbounded or silently dropping queue, a `common/runtime` composition owner, or imported SoftPC code. Stop and report if the inventory reveals an existing unclassified duplicate production route outside S1's documentation scope. |
| Exit Criteria | The ledger gives every current route one truthful disposition and deletion receiver; the common dependency graph is implementable with `common` depending only on `lib`; Core-debug is independently lifecycle-complete and bound only through later `vm/machine`; all listed queue semantics and run-id rules are finite; no implementation changes occurred; complete unit, documentation governance and diff checks pass. |
| Original Owner Request | Owner requires NXVM `core/debug` to be a separate component with its own queue, API and complete lifecycle; SoftPC does not require it now. Owner then approved admission of the next T. |
| Similar-Issue Sweep | S1 inventories all current NXVM Core/VM product-control, executor, presentation and host-facing routes, not merely the first route selected for later migration. It also compares every analogous SoftPC control/runtime/reconciler route read-only. Any current duplicate is recorded with its owner and later S deletion receiver; S1 may not patch a local instance. |

## Current Technical Baseline

- `vm-0-5-0526` remains the current target. Its stripped Release developer
  artifacts are `nxvm_0_5_0526_x64.exe` and `nxvm_0_5_0526_x86.exe` until a
  later implementation S creates a replacement task target.
- `src/lib` remains byte-identical to the canonical SoftPC corpus at the
  reference baseline. T527 S1 must not modify it.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance
