# Project Status

## Current Work

M5 T540 is closed. S1-S92 are accepted; S93 was superseded without acceptance.
S94-S96 supplied accepted review/verification results, and S97 delivers the
complete receiver and closes the original extraction request. No S is active.
No queued implementation has been admitted automatically.

Shared implementation P1 `4f2511b13` and NXVM implementation P2 `c83d3a252`
are pushed to origin/master. Coordinator review accepts the actual committed
tree: all original structural ledger members have their sole receiving owner,
required coverage/independent linkage remains intact, and other-App inputs
are unchanged. This closure changes no hardware timing grade.

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine.
- Flat `x86/ibmpc-common`, `ibmpc-at` and `ibmpc-xt` own shared/family board
  mechanisms. Genuine D4 remains Model40-owned.
- NXVM retains four fixed implemented products: XT, AT, Model40 and default.
  PC110 is not runnable; the four-App split remains [queued](QUEUE.md).
- Eight optimized stripped 0540 EXEs remain under their `assets/nxvm/<profile>`
  directories, with the runtime debugger and unchanged owner INIs. MyNES
  retains its unchanged 0043 pair and does not link the extracted x86 targets.

## Acceptance Evidence

[S94](../etc/evidence/t540-s94-source-review.md) records 626-path source/coverage
review and full 492/492 units per width. [S95](../etc/evidence/t540-s95-independent-verification.md)
records independent tools-on 298/298 and tools-off 292/292 per width.
[S96](../etc/evidence/t540-s96-artifacts-and-performance.md) records eight
artifact identities and once-only boot checkpoints.
[S97](../etc/evidence/t540-s97-delivery-review.md) reconciles all 58 integration
rows, final/pushed-tree dual-width gates, six manifests, exact artifact hashes,
scope/code-size review and build/test cost improvements. No owner INI, MyNES,
root README or shared-rule change is included.

## Historical Context

[T540 history](../history/M5-T540-shared-ibmpc-integration.md) records delivery
and acceptance. The [receiver work record](../etc/evidence/t540-s93-whole-board-receiver-work.md#legacy-current-snapshot)
preserves the former accumulated status as historical evidence, not another
status authority. [Queue](QUEUE.md) owns remaining candidates.
