# Project Status

## Current Work

M5 T542 S4 is admitted for input/display conversion after accepted S3.
The four-App split remains queued and depends on complete T542 acceptance.

| Task | Progress |
| --- | --- |
| T542 S4 | Active: Shared keyboard/mouse and direct snapshot-to-Common-frame conversion. S1-S3 accepted; T remains open. |

### Active Subtask Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T542 S4 after accepted S3 cdb768907; next unused sequential S. |
| Admission And Approval | Owner approved T542 and automatic sequential S admission. Coordinator admits S4 on 2026-10-04: Shared x86 source/tests/build and NXVM direct consumers/docs/artifacts only. |
| Objective | Consume the complete input/display ledger batch: one shared scan-set/mouse mapper and direct copied video-snapshot to Common-frame conversion; remove redundant production frame carriers. |
| Non-goals | No Lib/Common/MyNES/INI change, new native API, queue, input protocol, video state, font/geometry/timing behavior, profile registry, or App split. |
| Reference Baseline | Accepted S3 cdb768907; original mapping tables, CP437, 8x16 glyph/cursor scaling, frame bounds and display cadence/dirty predicates are the preservation baseline. |
| Candidate Proposal | [T542 proposal](../proposals/m5-shared-pc-machine-adapter.md), S4; [finite ledger](../history/M5-T542-shared-pc-machine-adapter.md), input/display batch. |
| Files And ABI Surface | Move keyboard/mouse mapper C/H and frame conversion plus owner-local tests to flat x86/product/machine public interfaces. Conversion consumes existing x86_video_snapshot and explicit presentation sequence, returning existing common_machine_frame. Remove production display-event/guest-frame carriers; existing guest-frame test view moves into test support. App display cadence/capture orchestration remains with its real Machine owner until cohesive S5 migration, calling the sole Shared converter directly. Repair all direct callers, gates, source lists and manifests. |
| Applicable Rules | Guide reading set, shared Execution/Architecture/Coding/Document, NXVM Architecture/Source Layout/source policy. Opaque/copy-only cross-owner operations, unique video/input owner, no App include in Shared, separate Shared/NXVM P targets. |
| Verification | Complete repository units x64/x86; independent mapper/frame tests and x86 corpus/negative/source-and-test manifests; six manifest checks, documentation and diff gates; eight optimized stripped 0542 profile EXEs with width/identity/hash proof. Preserve 58 integration contexts for S8 once-only acceptance. |
| Expected Markers | Both full unit suites pass; standalone conversions build without App; old mapper/frame paths and production redundant display carriers absent; unchanged snapshot-to-frame text/graphics/palette/font/cursor/CP437 semantics and cadence tests; eight verified products. |
| Asset Needs | Existing approved embedded firmware inputs only; no external master or INI mutation. Unit fixtures remain code-owned. |
| Reporting Requirements | Confirm receiver/preservation boundary, report actual conversion-path simplification, complete verification, counted C/H added/removed/net lines and scoped P delivery; coordinator reviews actual changes before acceptance. |
| Stop Conditions | Required Lib/Common/MyNES edit, changed input/video/timing capability, weakened boot/test predicate, native handle exposure or new queue/second guest state; report rather than silently changing scope. |
| Exit Criteria | All input/display conversion ledger members have one Shared receiver or explicitly temporary cohesive S5 orchestration owner; redundant production carriers removed, original assertions and behavior preserved, tests/artifacts/gates complete, scoped commits pushed and actual-diff acceptance recorded. |
| Original Owner Request | Extract remaining four-PC public mechanisms before independent App split; preserve real board/profile differences and minimize code/ownership complexity. |
| Similar-Issue Sweep | Scan every mapper/frame caller, production transport structure, display cadence/generation acknowledgment, cursor/font/palette/CP437 conversion, owner-local test and build/tool gate together. |


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
  remaining Machine/profile adapter extraction is still open.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
then-current 0541 artifact identities; S5 confirms that historical baseline.
Fresh S2/S3 units, independent helper/media/gate proofs and 0542 artifact hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
