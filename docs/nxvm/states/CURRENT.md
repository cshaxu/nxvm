# Project Status

## Current Work

M5 T539 remains open. S1-S9 are accepted; no S packet is active between
batches. The owner's 2026-09-28 authorization automatically admits subsequent
S work without another approval prompt.

| Task | Progress |
| --- | --- |
| T539 | PIT, RTC, PIC, DMA, AT keyboard chain and XT PPI/keyboard extraction are accepted. FDC is next; HDC, video, CPU/FPU and final finite-ledger review remain. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope and
decisions. Automatic admission does not waive boundary review, target-separated
commits, full verification or coordinator actual-diff acceptance.

M5 Td S174 queued this three-stage migration. Its first candidate is T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are current with
unchanged owner INI contents. MyNES retains its two unchanged 0043 receivers;
its T43 remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, the AT
keyboard chain at eb1e2e208, and XT PPI/keyboard at 0f9c6b1a8.
Current NXVM source/artifacts are 31e759965. [S9 evidence](../etc/evidence/t539-s9-xt-extraction.md)
owns source/test mapping, negative controls, verification and artifact hashes.

Coordinator review accepts the actual Shared and NXVM diffs: unique opaque
owners, board-only wiring, failed admission/rollback, scan/BAT/inhibit/release
ordering, original-case mapping, independent dependencies and all receivers.
No item remains in the bounded S9 packet; its packet is removed. Both
implementation P commits are pushed to origin/master. T539 is not closed.

Final units pass 347/347 per width, independent chip suites 18/18 per width,
and default external integration 20/20 per width. Every other profile/width
boot passes once. The specialized static aggregate, six manifests,
documentation governance, local links and whitespace checks pass.
Both reusable NXVM build trees are restored to default configuration.
No other product or sibling repository changed. Cooked-history rollback debt
remains in [TODO](TODO.md). Acceptance does not claim complete 8255 silicon,
new timing grades or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification. [S8 evidence](../etc/evidence/t539-s8-kbc-extraction.md)
retains the AT chain; [S7 evidence](../etc/evidence/t539-s7-dma-extraction.md)
retains DMA extraction and [S6](../etc/evidence/t539-s6-dma-first-service.md)
its first-service repair.
