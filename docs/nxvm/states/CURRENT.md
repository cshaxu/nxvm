# Project Status

## Current Work

M5 T539 remains open. S1-S16 are accepted. There is no active S packet.
The owner's automatic-S authorization remains; the next CPU batch requires
its concrete boundary review and admission before implementation.

## Retained Progress

| Task | Progress |
| --- | --- |
| T539 S16 | Accepted: Shared P1 5fa831a2b and NXVM P2 9783297fb extract the sole opaque FPU, retain CPU pairing/operand ownership and remove the old implementation. Both widths pass 368/368 units and 20/20 default integrations; tools-off 44/44, six manifests, specialized/document gates and eight artifact identities pass. |

Coordinator actual-commit review accepts the complete three-file FPU ledger
row. Arithmetic/timing bodies retain original algorithms; CPU pairing, opaque
lifetime, original-case migration, public deadline observations, receiving
builds and target-separated commits were reviewed directly. The paused-frame
integration correction preserves every cell assertion. Model40's initial
instrumented timeouts are retained: one controlled contrast per width with
only the existing retirement observer disabled reached installer-ready under
the unchanged 90-second bound. No guest workaround, timing downgrade or new
full-x87 claim was introduced.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S16 boundary](../etc/architecture/t539-s16-fpu-extraction.md),
[S16 evidence](../etc/evidence/t539-s16-fpu-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, limits and delivery.
CPU and all remaining inventory dispositions still belong to T539; none is
silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
9783297fb with unchanged owner INIs. S16 evidence records hashes, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba, video at
522d0b27f and FPU at 5fa831a2b. MyNES retains its unchanged 0043 pair: its link
inputs do not include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S16.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
