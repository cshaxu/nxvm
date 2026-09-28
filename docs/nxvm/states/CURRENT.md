# Project Status

## Current Work

M5 T539 remains open. S6's DMA first-service repair is accepted after actual-change
review; opaque DMA extraction is the next batch. No S is currently active.
Subsequent bounded batches continue under the owner's automatic-S authorization
dated 2026-09-28; no additional manual admission is required.

| Task | Progress |
| --- | --- |
| T539 S6 | Accepted: 0746220bf pushed; actual-change review and all bounded exit criteria pass. Eight 0539 products are current. DMA extraction and the remaining finite inventory stay open. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
decisions and acceptance. Automatic admission does not waive evidence,
target-separated commits, regression testing or coordinator review.

M5 Td S174 queued the three-stage migration. Its first candidate is now T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized, compiler-debug-stripped 0539 EXEs are current with
unchanged owner INIs. MyNES retains its two unchanged 0043 receivers; its T43
remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648 and PIC at d6dc6ca3a. Current NXVM source/artifacts
are 0746220bf, including the first-service repair and eight rebuilt artifacts. [S6 evidence](../etc/evidence/t539-s6-dma-first-service.md)
owns current verification/hashes; [S5 evidence](../etc/evidence/t539-s5-pic-extraction.md)
retains PIC source mapping and transaction rollback. Full sibling parity is
not claimed; no sibling repository was modified.

Verification: NXVM 341/341 units and 20/20 default-profile external integration
per width; all six non-default profile/width boot matrices pass once.
S6's 126 DMA first-service cases pass within the unit suite. Independent chip
suites remain S5's 12/12 evidence, not a new standalone S6 run. The specialized
static aggregate and six manifests pass. MyNES has no artifact input change.
The S5 temporary build trees are removed; the two S3 NXVM incremental trees
remain for the immediately next chip batch. Cooked-history rollback debt
remains in [TODO](TODO.md). This bounded regression acceptance does not claim
complete hardware qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification and final accepted single-pass matrix.
