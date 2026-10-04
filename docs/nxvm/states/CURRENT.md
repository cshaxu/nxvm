# Project Status

## Current Work

M5 T542 S5 is admitted: isolate Profile-owned observations before cohesive
execution/debug extraction. The four-App split remains queued behind T542.

| Task | Progress |
| --- | --- |
| T542 S4 | Accepted: Shared input/frame conversion and eight products delivered; redundant production frame carriers removed. S1-S4 accepted; T remains open. |

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T542 S5; follows accepted S4 at 6ee21f50a; numeric S only. |
| Admission And Approval | Owner approved T542 and automatic sequential admission; coordinator admits the existing Model40-isolation prerequisite on 2026-10-04. NXVM target only; no new product behavior or exception. |
| Objective | Move D4 borrowed handle and FDC terminal observation into the actual prepared Profile context; generic Machine owns neither. Preserve probe/reset/failure contracts. |
| Non-goals | No Lib/Common/Shared source, MyNES, INI, external master, timing/CPU/hardware behavior changes; no new registry or worker. Execution/debug relocation remains S6, fixed composition S7; complete task remains required. |
| Reference Baseline | Clean 6ee21f50a; T542 S1 construction/lifetime ledger and S4 evidence; existing Model40 observation consumers. |
| Candidate Proposal | [T542](../proposals/m5-shared-pc-machine-adapter.md), revised linear sequence retains the full extraction/acceptance scope. |
| Files And ABI Surface | app-nxvm/profiles/machine_plan and copied Model40 observation contract; machine.c/private/lifecycle; corresponding App unit/integration consumers and static gates; eight NXVM 0542 artifacts and evidence. |
| Applicable Rules | Guide, CONTRIBUTING, EXECUTION, DOCUMENT, ARCHITECTURE, CODING, NXVM architecture/layout and source policy; architecture/coding skills. Unique Profile state owner, copied public boundary, Core attachment teardown, unchanged predicates and target-scoped commits. |
| Verification | Incremental builds then complete unit CTest -L unit -j12 on t542-s2-unit-x64/x86; Model40 observation/reset and retirement fixture assertions; all six manifests, dependency/document gates, diff review; eight Release vm-0-5-0542 builds, PE/identity/no-debug/hash checks. Full 58 external contexts remain once-only T acceptance S9, not claimed here. |
| Expected Markers | Both full unit suites pass; no model40 state in generic Machine; old observer sink absent; all original probe predicates retained; four pairs current and INIs unchanged. |
| Asset Needs | Existing build-authorized embedded firmware inputs only; no new acquisition/writes to external masters; units code-owned. |
| Reporting Requirements | Confirm boundary; report ownership/proof progress; executor evidence then coordinator actual-diff acceptance and immediate scoped push. |
| Stop Conditions | Need for excluded Shared/Lib/Common edits, hardware semantics/grade change, weakened original predicate or new asset assumption; report rather than bypass. |
| Exit Criteria | All Model40 state removed from generic Machine; sole Profile context owns copied observation and reset/teardown validity; all direct consumers/gates repaired, full units and current dual-width artifacts delivered, actual changes reviewed and pushed. |
| Original Owner Request | Again extract IBM-PC public components; independent chips/board/Product mechanisms shared before four independent Apps, without patch layering or duplicate owners. |
| Similar-Issue Sweep | Search model40_board, model40_fdc_terminal, FDC observation sinks and materialize/reset consumers across src/test/cmake/tools; classify every hit, forbid return of profile fields in generic Machine with an App gate. |


## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine. Flat `x86/ibmpc-common`, `ibmpc-at` and
  `ibmpc-xt` own common/family board mechanisms; genuine D4 remains Model40-owned.
- `x86/product` owns the one PC INI/request/startup, command/hotkey/Debug
  binding, Common composition and process entry/banner. App retains immutable
  identity, fixed config/factory and its real Machine/profile/firmware composition.
- NXVM retains four fixed implemented products under app-nxvm: XT, AT, Model40
  and default. PC110 is not runnable. The independent App split is queued.
- Eight optimized stripped 0542 EXEs reside only in `assets/nxvm/<profile>/`,
  with the runtime debugger and unchanged owner INIs. MyNES retains its unchanged
  0043 pair and does not link Product. Lib/Common remain unchanged. S2 moves
  common floppy, ROM validation and contract checks into `x86/ibmpc-common`;
  S3 moves media providers to x86/product/machine/media with opaque App handles;
  S4 owns pure input/frame conversion there and removes redundant frame carriers;
  S5 isolates Model40 observations at the prepared Profile context;
  remaining Machine/profile adapter extraction is still open.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
then-current 0541 artifact identities; S5 confirms that historical baseline.
Fresh S2-S4 units, independent helper/media/conversion proofs and 0542 hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
