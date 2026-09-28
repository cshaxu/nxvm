# Project Status

## Current Work

M5 T539 remains open. S7's DMA extraction is accepted after actual-change
review; there is no active S packet between batches. The next bounded batch
is the AT keyboard/controller chain, following the approved dependency order.
The owner's 2026-09-28 automatic-S authorization remains in force; no further
manual admission is required within the approved scope.

| Task | Progress |
| --- | --- |
| T539 | S1-S7 closed. PIT, RTC, PIC and DMA have single opaque Shared implementations and verified NXVM board adapters. AT keyboard chain, XT PPI/keyboard, FDC, HDC, video, CPU/FPU and final finite-ledger review remain; none is implicitly accepted. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
decisions and acceptance. Automatic admission does not waive boundary review,
target-separated commits, complete tests or coordinator actual-diff review.

M5 Td S174 queued the three-stage migration. Its first candidate is T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized, compiler-debug-stripped 0539 EXEs are current with
unchanged owner INIs. MyNES retains its two unchanged 0043 receivers; its T43
remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648, PIC at d6dc6ca3a and DMA at 53b4be21d. Current
NXVM source/artifacts are 217125697. [S7 evidence](../etc/evidence/t539-s7-dma-extraction.md)
owns verification/hashes and actual-change acceptance. [S6 evidence](../etc/evidence/t539-s6-dma-first-service.md)
retains the DMA first-service prerequisite; [S5 evidence](../etc/evidence/t539-s5-pic-extraction.md)
retains PIC mapping/rollback. Full sibling parity is not claimed; no sibling
repository was modified.

Verification: final NXVM units 342/342 per width, default-profile external
integration 20/20 per width, independent chip suites 13/13 per width. All six
remaining profile/width boot rows pass once. The specialized static aggregate,
six manifests, documentation governance and diff checks pass. MyNES has no
artifact input change. The temporary S7 standalone chip build trees are removed;
the two S3 NXVM incremental trees remain, restored to default configuration,
for the immediately next chip batch. Cooked-history rollback debt remains in
[TODO](TODO.md). This bounded acceptance does not claim complete hardware
qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification and final accepted single-pass matrix.
