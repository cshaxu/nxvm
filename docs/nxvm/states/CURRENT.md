# Project Status

## Current Work

M5 T542 S4 is accepted. Execution/debug adapter extraction is the next planned
batch; the four-App split remains queued behind complete T542 acceptance.

| Task | Progress |
| --- | --- |
| T542 S4 | Accepted: Shared input/frame conversion and eight products delivered; redundant production frame carriers removed. S1-S4 accepted; T remains open. |


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
