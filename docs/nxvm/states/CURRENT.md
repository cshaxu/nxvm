# Project Status

## Current Work

M5 T539 remains open. S1-S15 are accepted; no S packet is active between
this delivery and the next automatic boundary-reviewed admission.

| Task | Progress |
| --- | --- |
| T539 S15 | Accepted: Shared P1 522d0b27f and NXVM P2 88ae417ff move the sole video owner and reconnect board routes. Final units 367/367 and default integrations 20/20 per width; six vendor boots once; tools-off 43/43; six manifests, specialized/document gates and eight artifact hashes pass. |

Coordinator actual-commit review accepts the complete video/display ledger
batch: sole register/VRAM/frame ownership, typed memory and copied diagnostic
boundary, unchanged original scenario expectations, atomic registration/retry,
optional VGA route preservation and explicit retirement of unused presentation
helpers. The review inspected source/test/build/document changes, not only
green checks. Post-delivery standalone checks pass 43/43. There is no new
complete-silicon or timing-grade claim.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S15 boundary](../etc/architecture/t539-s15-video-extraction.md),
[S15 evidence](../etc/evidence/t539-s15-video-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, code-size review and delivery.
CPU/FPU and all remaining inventory dispositions still belong to T539.
Their dependency/boundary review precedes each next automatic S admission;
none is silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
88ae417ff with unchanged owner INIs. S15 evidence records SHA-256, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba and video
at 522d0b27f. MyNES retains its unchanged 0043 pair: its link inputs do not
include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S15.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
