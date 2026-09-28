# Project Status

## Current Work

M5 T539 remains open. S4 RTC extraction is closed after actual-change review;
no S is active at this acceptance boundary. The owner's automatic-S admission
authorization remains effective; the next bounded chip batch is PIC.

| Task | Progress |
| --- | --- |
| T539 | PIT and RTC accepted; remaining finite chip inventory pending. |

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
at 24162ac93; RTC at 8a8435648. Current NXVM source/artifacts are 06f99605d.
[S4 evidence](../etc/evidence/t539-s4-rtc-extraction.md) owns the source mapping,
artifact hashes and malformed-month safety evidence. Full sibling parity is
not claimed; no sibling repository was modified.

Verification: NXVM 339/339 units and 20/20 default-profile external integration
per width; all six non-default profile/width boot matrices pass once.
Independent chip suites are 10/10 per width; all 93 specialized static steps
and six manifests pass. MyNES has no x86 dependency or artifact input change.
The S4 temporary build trees are removed; the two S3 NXVM incremental trees
remain for the immediately next chip batch. Cooked-history rollback debt
remains in [TODO](TODO.md). This bounded regression acceptance does not claim
complete hardware qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification and final accepted single-pass matrix.
