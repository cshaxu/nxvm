# Project Status

## Current Work

M5 T542 is reopened for residual composition extraction. S11's package
relocation remains accepted; it did not finish every construction/factory
member of the original inventory. The four-App split remains queued and
unadmitted until the corrective ledger is exhausted.

| Work | Progress |
| --- | --- |
| T542 S17 | ROM preparation classification and consolidation admitted; S18-S19 remain planned. |

S12 delivery `0d8c3d712` defines the [corrective ledger and sequence](../proposals/m5-pc-composition-completion.md).
S13 delivers Shared `05ca27ff2` and NXVM `c7a296138`; its coordinator review
accepts the neutral contracts row. S14 delivers Shared `07820fd58` and NXVM
`277b0800d`; coordinator review accepts the factory/Product row only. S15
Shared `778f6b2f5` and NXVM `0ff7726d7` are accepted after actual-change review.
Candidate finishing and three-consumer AT construction are closed. The owner's
automatic sequential-S approval covers the remaining plan; T542 remains open.

## Next Work

S16 is accepted after separate coordinator review of Shared e2df2d0d4 and
NXVM 66eb9c1cb. S17 must classify/consolidate ROM preparation; S18 must finish
selected-build/embed/deploy ownership. S19 retains whole-ledger actual-diff
review and all 58 external integration contexts once before T542 closure.
Each numeric S requires its own admission packet before implementation.

## Active S17 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T542 S17 after accepted S16 2dbd26280; next unused numeric S. |
| Admission And Approval | Owner reopening plus automatic sequential-S approval, 2026-10-04. Shared ibmpc and NXVM four fixed products are allowed; one target per P, immediate origin/master push. |
| Objective | Consume the ROM ledger row: classify all four preparation/mapping paths; consolidate identical even/odd interleave in board-common and bounded byte copies through existing Machine preparation; preserve distinct model layouts and candidate lifetime. |
| Non-goals | No Lib/Common/x86 algorithms, MyNES, timing, firmware behavior, owner INIs, external assets, App split or build-selection cutover. No new ROM registry, backing store, host BIOS service or generic loader framework. |
| Reference Baseline | Clean 2dbd26280; accepted S16 with 503/503 units per width and eight current optimized stripped 0542 EXEs. |
| Candidate Proposal | [Composition completion](../proposals/m5-pc-composition-completion.md), ROM preparation ledger row and S17. |
| Files And ABI Surface | Existing board-common ROM validation adds one checked byte-interleave operation; default/5170 and Model40 consume it. XT/default/Model40 copy callers use existing Machine asset preparation. App retains chip sizes, regions, alias declarations, optional ROM eligibility, allocations and provider/reset policy. Tests/gates/manifests and eight EXEs follow actual owners. |
| Applicable Rules | Task Reading Set; Execution, CONTRIBUTING, shared Architecture/Coding/Document, NXVM Architecture/Coding/source policy; architecture-governance then coding-governance skills. One copied candidate owner, no Shared-to-App edge or second ROM backing; exact dependency/source review and failure/layout regressions prove these invariants. No import or exception. |
| Verification | Full run-unit-tests and verify-current-specialized-gates in build/t542-s13/unit-x64 and unit-x86; standalone test/ibmpc ROM regression/ibmpc-verify; independent manifests/corpus/negative/DAG checks; eight retained Release vm-0-5-0542 builds; PE/stripping/hash; documentation governance and git diff --check. All 58 external integration contexts remain once-only S19 acceptance. |
| Expected Markers | Full dual-width unit/gates pass; original ROM reset/alias/read-only checks preserved; interleave has no App duplicate; malformed input leaves destination untouched and construction publishes no failed candidate; all eight artifacts current. |
| Asset Needs | Existing compiled immutable firmware build inputs only. Unit/regression bytes are code-owned; no external master, media, ROM acquisition or INI edit. |
| Reporting Requirements | Confirm concrete shared/retained classification before implementation; record exact commands/results, actual source/test counts, all mapping differences, failure proof, code review, commit identities and eight hashes; coordinator independently reviews actual diff. |
| Stop Conditions | Required excluded edit, changed valid ROM bytes/regions/aliases or firmware semantics, lost original predicate, second lifetime/backing owner or reverse dependency. Bounded malformed-input copying errors are in scope and must be covered, not hidden as extraction. |
| Exit Criteria | One checked even/odd interleave implementation used by default/5170/Model40, original output order/layout identical. Exact-size copy remains one existing mechanism; optional video copies only validated supplied bytes, never a maximum beyond source length. Every retained provider/allocation/layout path has a concrete semantic reason and regression; no generic loader copied per future App. Full required verification/artifacts/push and coordinator audit complete. |
| Original Owner Request | Correctly separate all four models' shared logic into x86/ibmpc before App splitting; share mechanisms, retain real board-specific definitions, automatically admit sequential numeric S tasks. |
| Similar-Issue Sweep | Inspect all profiles/*/machine_plan.c and ROM providers, ROM validation/Machine preparation, firmware/register/alias callers, all direct memory_copy/interleave loops and their unit/static/build callers. Default/5170 flat or chip image, Model40 chip image/aliases/video prefix exclusion and XT system/Xebec/variable video each receive explicit disposition. Check bounded video lengths and allocation/rollback in every constructor. |

## Retained Runnable Evidence

S16's source and eight rebuilt 0542 artifacts pass complete units 503/503 per
width, independent package checks, specialized and documentation gates. The
[T542 evidence](../history/M5-T542-shared-pc-machine-adapter.md) records source
deliveries, actual-diff review, counts and current hashes. S11's 58 once-only
optimized integration passes remain historical; they are not fresh S15 results.
Whole-task S19 must run all 58 contexts once on the completed extraction.
Lib/Common, MyNES, INIs and external masters remain unchanged. The Common
wake-failure contract and Shared vocabulary follow-up remain in [TODO](TODO.md),
not authorized or claimed repaired by this relocation.

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine. `ibmpc/board-common`, `ibmpc/board-at` and
  `ibmpc/board-xt` own common/family board mechanisms; genuine D4 remains Model40-owned.
- `ibmpc/product` owns the one PC INI/request/startup, command/hotkey/Debug
  binding, Common composition and process entry/banner. App retains immutable
  identity, fixed composition binding and genuine Profile/firmware preparation.
- NXVM retains four fixed implemented products under app-nxvm: XT, AT, Model40
  and default. PC110 is not runnable. The independent App split is queued.
- S13 places neutral construction config/assets in ibmpc/machine without a
  model ID. App owns fixed constructor declarations and Model40 observations.
  S16 moves immutable AT grammar to board-at and its copied contract projection
  to board-common; default, 5170 and Model40 independently supply inputs.
- Eight optimized stripped 0542 EXEs reside only in `assets/nxvm/<profile>/`,
  with the runtime debugger and unchanged owner INIs. MyNES retains its unchanged
  0043 pair and does not link Product. Lib/Common remain unchanged. S2 moves
  common floppy, ROM validation and contract checks into `ibmpc/board-common`;
  S3 moves media providers to ibmpc/machine/media with opaque App handles;
  S4 owns pure input/frame conversion there and removes redundant frame carriers;
  S5 isolates Model40 observations at the prepared Profile context;
  S6 owns the complete execution/debug adapter and copied construction at
  ibmpc/machine; S7 binds fixed constructors without the App plan union/runtime dispatch.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
then-current 0541 artifact identities; S5 confirms that historical baseline.
Fresh S2-S11 units, helper/media/conversion/Profile proofs and current 0542 hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
