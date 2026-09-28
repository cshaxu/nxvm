# Project Status

## Current Work

M5 T539 remains open. S3 is closed after the approved PIT extraction and
complete NXVM reconnection; no next source batch is admitted.

| Task | Progress |
| --- | --- |
| T539 S3 | Closed: Shared PIT and all NXVM receivers verified, committed and pushed; remaining chips await their next boundary review. |

[Previous task history](../history/M5-T538-deployed-boot-pairs.md) retains reviewed packets
and actual-change acceptance. [Archived proposal](../history/M5-T538-deployed-boot-pairs-proposal.md),
[convergence ledger](../etc/evidence/t538-boot-pairs.md), and
[S7 evidence](../etc/evidence/t538-s7-orphan-release.md) record scope, revised
acceptance, tests and hashes.

M5 Td S174 queued the three-stage migration. Its first candidate is now T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized, compiler-debug-stripped 0539 EXEs replace 0538 with
unchanged owner INIs. MyNES retains its two unchanged 0043 receivers; its T43
remains closed.

Lib/Common retain the accepted 268464d49 baseline, including the prior orphan
release repair and test-path corrections. Shared x86 now adds the S3 PIT
component at 24162ac93; NXVM source/artifacts are 797ad8887.
[S3 evidence](../etc/evidence/t539-s3-pit-extraction.md) owns the
source/artifact mapping. Full sibling parity is not claimed, and no sibling
repository was modified.

Verification: NXVM 337/337 units and 20/20 default-profile external integration
per width; all six non-default profile/width boot matrices pass once. Independent
PIT is 8/8 per width; all 66 specialized static steps and six manifests pass.
MyNES is not a PIT receiver and has no artifact input change. Cooked-history
rollback debt remains in [TODO](TODO.md). This bounded regression acceptance
does not claim complete hardware timing qualification or indefinite absence of
intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. T538 history preserves S6's negative
qualification and S7's unclassified observations alongside the final accepted
single-pass matrix.
