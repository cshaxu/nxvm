# Project Status

## Current Work

M5 T539 remains open. S1-S8 are accepted; there is no active S packet at this
acceptance boundary. The owner's 2026-09-28 automatic-S authorization remains
in force. The next bounded batch is XT PPI/keyboard; it needs its concrete
boundary review and packet, not another manual admission.

| Task | Progress |
| --- | --- |
| T539 | PIT, RTC, PIC, DMA and the AT controller/keyboard/AUX chain are extracted and accepted. XT PPI/keyboard, FDC, HDC, video, CPU/FPU and final finite-ledger review remain; none is implicitly accepted. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope and
decisions. Automatic admission does not waive boundary review, target-separated
commits, complete tests or coordinator actual-diff acceptance.

M5 Td S174 queued this three-stage migration. Its first candidate is T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are current with
unchanged owner INI contents. MyNES retains its two unchanged 0043 receivers;
its T43 remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d and the AT
keyboard chain at eb1e2e208. Current NXVM source/artifacts are 6ae9802dc.
[S8 evidence](../etc/evidence/t539-s8-kbc-extraction.md) owns source/test mapping,
verification, hashes and actual-change acceptance. S8 reviewed the real chip,
board, test, diagnostic, build and document diffs; no item remains in its
bounded brief. Shared and NXVM deliveries are pushed to origin/master.

Final units pass 345/345 per width, independent chip suites 16/16 per width,
and default external integration 20/20 per width. Every other profile/width
boot passes once. The specialized static aggregate, six manifests,
documentation governance, 50 local links and whitespace checks pass.
Both reusable NXVM build trees are restored to the default configuration.
S8 standalone chip trees remain available for the next batch. No other product
or sibling repository changed. Cooked-history rollback debt remains in [TODO](TODO.md).
This bounded acceptance does not claim new hardware qualification or
indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification. [S7 evidence](../etc/evidence/t539-s7-dma-extraction.md)
retains the DMA extraction, [S6](../etc/evidence/t539-s6-dma-first-service.md)
its first-service repair, and [S5](../etc/evidence/t539-s5-pic-extraction.md)
the PIC mapping/rollback proof.
