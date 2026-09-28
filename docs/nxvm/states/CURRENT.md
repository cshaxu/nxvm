# Project Status

## Current Work

M5 T539 remains open. S1-S12 are accepted. There is no active S packet;
the next automatic admission is FDC extraction after its concrete boundary
review. The owner's automatic-S authorization remains in force.

| Task | Progress |
| --- | --- |
| T539 S12 | Accepted: P1 408a31cc7 supplies qualified FDC drive/status behavior, recovered BIOS source and one embedded-ROM route. Full units 350/350 per width; default integrations 20/20 per width and vendor boots once per width. Eight EXEs, static gates and manifests verified. FDC extraction remains pending. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope and
decisions. Automatic admission does not waive boundary review, target-separated
commits, full verification or coordinator actual-diff acceptance.

S10 [decision record](../etc/architecture/t539-s10-fdc-boundary.md) and
[evidence](../etc/evidence/t539-s10-fdc-characterization.md) record 120 new
characterization cases and 347/347 full units per width. Intel READY/SEEK
contradictions and pending-completion safety remain explicit cutover gates;
no production, Shared or artifact change is claimed.

S11 [evidence](../etc/evidence/t539-s11-fdc-seek-ownership.md) records reproduced
identity/duplicate/overflow-admission failures, their single-owner correction,
347/347 units per width, 20/20 default integrations per width, each other
profile boot once and eight updated artifacts. Shared and INI are unchanged.

S12 [evidence](../etc/evidence/t539-s12-fdc-drive-status.md) records source
qualification, actual-diff acceptance, negative controls and packaging proof.
Commercial originals remain external; owner-approved embedded EXEs are tracked.

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
Current NXVM source/artifacts are 408a31cc7. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns qualification, firmware recovery, negative controls and artifact hashes.

Coordinator-role review accepts S12's actual source/build/test/document diff,
including the deleted runtime loaders and machine-specific unready policy,
guest-owned firmware services, source-qualified status changes and stopped-HLT
cancellation. P1 is pushed to origin/master; no S12 packet item remains. The
packet is removed. FDC extraction and the remaining ledger keep T539 open.

Final units pass 350/350 per width; default external integration passes 20/20
per width and every other profile/width boot passes once. The latter boot
evidence precedes the host-only stopped-HLT cancellation repair, as explicitly
qualified in S12 evidence; it is not claimed as a fresh boot of each final hash.
The post-commit specialized static aggregate, six manifests,
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
