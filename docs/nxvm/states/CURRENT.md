# Project Status

## Current Work

M5 T542 S1 is admitted for remaining shared PC Machine logic: inventory and
contract design before source migration. T541 remains accepted and closed
through S5 delivery 5249e4f1e. The four-App split remains queued, depends on
T542 and is not admitted or numerically allocated.

| Task | Progress |
| --- | --- |
| T542 S1 | Active: freeze Machine/helper receiver ledger and migration contracts; no production changes yet. |

## Active S Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T542 S1 after closed T541; planning prerequisite of approved implementation. |
| Admission And Approval | Human owner, 2026-10-04: admit a new T for shared logic extraction and show the S plan. NXVM and Shared are approved later code targets; S1 changes NXVM task/design records only. Lib/Common and MyNES changes excluded. |
| Objective | Freeze complete remaining PC Machine/helper receiver ledger, construction/lifetime contracts, dependencies, migration batches and verification map. |
| Non-goals | No source/test relocation, new App, behavior/timing change, registry, Lib/Common API, external asset or owner INI change; no claim of implemented extraction. |
| Reference Baseline | Closed T541 at 55f9fb1920af9316c6102b7aff5550fe1d7b6dfc; initial clean worktree; eight deployed 0541 EXEs unchanged. |
| Candidate Proposal | [Shared PC Machine adapter](../proposals/m5-shared-pc-machine-adapter.md); ledger in [T542 history](../history/M5-T542-shared-pc-machine-adapter.md). |
| Files And ABI Surface | docs/nxvm proposal/history/status/Queue/Roadmap and affected design/successor records; inspect App Machine/profile and Shared contracts with test/build/tool callers. Design minimum typed boundary; no ABI change in S1. |
| Applicable Rules | Execution New allocation, one active S, full-batch ledger, target-scoped P and actual-change review; Documentation authority/links; Architecture neutral dependency, unique owner, opaque/copied boundary and rollback; Coding cohesive files/no forwarding abstraction. Read docs/nxvm/README.md, CONTRIBUTING.md, design/ARCHITECTURE.md, design/CODING.md, design/ROADMAP.md and docs/rules/{EXECUTION,DOCUMENT,ARCHITECTURE,CODING}. Source policy for retained provenance only; no acquisition/import. |
| Verification | Exact source/caller/test/build inventory and receiver review; tools/shared/Verify-DocumentationGovernance.ps1 -RepositoryRoot . -Product nxvm; git diff --check; changed Markdown links and actual-diff/Git-numbering review. No changed runtime inputs: no S1 EXE rebuild or runtime-test claim. |
| Expected Markers | Complete ledger without unclassified member; no designed Shared-to-App/peer-App dependency; documentation governance passes; no whitespace error. |
| Asset Needs | Retain existing firmware/media identities; no binary loading, acquisition, path rewrite or deployment in S1. |
| Reporting Requirements | Confirm/raise objections; show numbered S plan; report concrete contracts, retained differences, evidence/gates and commit status before S1 acceptance. |
| Stop Conditions | Required Lib/Common changes, new behavior, new source/asset/license authority or reduced machine/test coverage: report and revise before affected implementation. |
| Exit Criteria | Scope mapped to exact owners/batches; minimum construction/failure contract frozen; test/build/tool/artifact consumers accounted for; documents consistent; gates and coordinator diff review accepted; complete S1 delivery committed/pushed. |
| Original Owner Request | Admit a new T to extract remaining shared logic and provide its S breakdown before four-App split. |
| Similar-Issue Sweep | All App Machine/media/profile-helper/factory dependencies and corresponding test/build/tool references: classify shared mechanisms versus concrete differences. No production repair in S1; no shared path left unclassified. |

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine. Flat `x86/ibmpc-common`, `ibmpc-at` and
  `ibmpc-xt` own common/family board mechanisms; genuine D4 remains Model40-owned.
- `x86/product` owns the one PC INI/request/startup, command/hotkey/Debug
  binding, Common composition and process entry/banner. App retains immutable
  identity, fixed config/factory and its real Machine/profile/firmware composition.
- NXVM retains four fixed implemented products under app-nxvm: XT, AT, Model40
  and default. PC110 is not runnable. The independent App split is queued.
- Eight optimized stripped 0541 EXEs reside only in `assets/nxvm/<profile>/`,
  with the runtime debugger and unchanged owner INIs. MyNES retains its unchanged
  0043 pair and does not link Product. Lib/Common and existing x86 implementations
  were not changed by T541.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
current artifact SHA-256 identities; S5 confirms they are unchanged.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
