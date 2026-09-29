# Project Status

## Current Work

M5 T539 remains open. S1-S14 are accepted. No S packet is active between
this acceptance and the next automatic admission.

| Task | Progress |
| --- | --- |
| T539 S14 | Accepted: Shared P1 9020d8bba and NXVM P2 86e0f82cb extract the sole opaque HDC family and reconnect board/media wiring. Units 358/358 and default integrations 20/20 per width; six vendor boots once; tools-off 25/25; six manifests, static/document gates and eight artifact hashes pass. |

Coordinator actual-commit review accepts the complete HDC ledger row: explicit
ATA/Compaq/WD1003/Xebec personalities, one state/sector-buffer owner, absolute
deadlines, output withdrawal, large-sector addressing, checked construction
and rollback, original-case migration and paused terminal diagnostics.
The review inspected actual source/test/build/document changes, not only green
checks. There is no new complete-silicon or timing-grade claim.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S14 boundary](../etc/architecture/t539-s14-hdc-extraction.md),
[S14 evidence](../etc/evidence/t539-s14-hdc-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, code-size review and delivery.
CPU/FPU, video and all remaining inventory dispositions still belong to T539.
Their dependency/boundary review precedes each next automatic S admission;
none is silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
86e0f82cb with unchanged owner INIs. S14 evidence records SHA-256, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876 and HDC at 9020d8bba.
MyNES retains its unchanged 0043 pair: its link inputs do not include these
x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S14.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
