# Project Status

## Current Work

M5 T539 remains open. S1-S13 are accepted. No S is active between this
acceptance and the next coordinator admission. The owner's automatic-S
authorization remains in force for subsequent bounded chip batches.

| Task | Progress |
| --- | --- |
| T539 S13 | Accepted: Shared P1 1d6dc5876 and NXVM P2 c18859898 extract the sole opaque 8272A mechanism and reconnect the PC adapter. Full units 355/355 per width; default integrations 20/20 per width; six vendor boots once; tools-off 22/22; static gates and six manifests pass. Eight 0539 EXEs are committed/pushed. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S13 boundary](../etc/architecture/t539-s13-fdc-extraction.md) and
[S13 evidence](../etc/evidence/t539-s13-fdc-extraction.md) retain scope,
coverage, actual-diff review, verification and artifact hashes.
[Task history](../history/M5-T539-independent-shared-chips.md) records delivery.

Coordinator-role actual-commit review accepts S13's chip state/command
ownership, board drive/media/port wiring, construction rollback, deadline
integration, copied diagnostics and original-case migration. The review
includes the recording-qualification and READY-trigger corrections, not merely
the passing tests. No FDC extraction item remains open in this bounded batch.
Existing flat-record, READ TRACK and timing limits are unchanged.

CPU/FPU, HDC, video and all remaining inventory dispositions still belong to
T539. Their dependency/boundary review precedes each next automatic S admission.
The full task is not complete, and none of those rows is silently transferred
to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs from the complete
S13 source delivery are current with unchanged owner INI contents.
[S13 evidence](../etc/evidence/t539-s13-fdc-extraction.md) records final SHA-256,
PE architecture, vendor boot checkpoints and default external integrations.
Both reusable NXVM build trees are restored to default configuration.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, the AT
keyboard chain at eb1e2e208, XT PPI/keyboard at 0f9c6b1a8 and FDC at 1d6dc5876.
NXVM source/artifacts are c18859898. MyNES retains its unchanged 0043 pair:
its executable link inputs do not include the changed x86 chip targets.

The owner-approved firmware packaging remains the S12 route: project BIOS
source lives in app-nxvm/firmware; commercial originals remain external BYOB;
all selected ROM bytes are embedded into the committed product EXEs.
No runtime ROM-file fallback, host BIOS service, external-asset or user-INI
change is introduced by S13.

The seven remaining [Queue](QUEUE.md) candidates retain dependency order.
Cooked-history rollback debt remains in [TODO](TODO.md). Acceptance does not
claim complete silicon qualification, upgraded timing grades or indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
