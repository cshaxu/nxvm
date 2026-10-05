# Project Status

## Current Work

M5 T542 corrective S11 is accepted and closed. The separate IBM PC package
completes the shared extraction boundary without changing implementation
behavior. No S is active; the four-App split remains queued and unadmitted.

Shared delivery is `51a6d209e`; NXVM callers, gates and eight current artifacts
are `8c24462b3`. Complete units pass 499/499 per width, all 58 once-only optimized
integration contexts pass, and independent package, manifest, specialized and
documentation checks pass. The [S11 evidence](../history/M5-T542-shared-pc-machine-adapter.md)
records actual-diff review, counts, intermediate failures and current hashes.
Lib/Common, MyNES, INIs and external masters remain unchanged. The Common
wake-failure contract and Shared vocabulary follow-up remain in [TODO](TODO.md),
not authorized or claimed repaired by this relocation.

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine. `ibmpc/board-common`, `ibmpc/board-at` and
  `ibmpc/board-xt` own common/family board mechanisms; genuine D4 remains Model40-owned.
- `ibmpc/product` owns the one PC INI/request/startup, command/hotkey/Debug
  binding, Common composition and process entry/banner. App retains immutable
  identity, fixed config/factory and genuine Profile/firmware composition.
- NXVM retains four fixed implemented products under app-nxvm: XT, AT, Model40
  and default. PC110 is not runnable. The independent App split is queued.
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
