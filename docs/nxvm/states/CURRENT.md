# Project Status

## Current Work

M5 T539 remains open. S5 PIC extraction is closed after actual-change review.
S6 repairs the DMA first-service phase bypass before opaque chip extraction.
Subsequent bounded batches continue under the owner's automatic-S authorization
dated 2026-09-28; no additional manual admission is required.

| Task | Progress |
| --- | --- |
| T539 S6 | Complete executor delivery: DMA first-service bypass removed, 126-case negative/positive control, full dual-width tests and eight products pass. Awaiting pushed actual-change review; DMA extraction remains pending. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S6 after S5 acceptance 4f278a918. |
| Admission And Approval | Owner authorized automatic admission of every S on 2026-09-28. Coordinator admits this NXVM-only DMA extraction prerequisite after source inspection found a first-transfer bypass. No Shared or MyNES source modification. |
| Objective | Remove the secondary-controller first-transfer bypass so all eligible channels enter the existing normal/compressed service phases before observable transfer effects; characterize the complete affected channel/mode family before extraction. |
| Non-goals | Extracting DMA yet, changing page/reset/cascade semantics, a new scheduler or timing model, new hardware-accuracy grades, BIOS/media/INI changes, Shared/MyNES edits or sibling writes. |
| Reference Baseline | 4f278a918; dma.c/dma.h ledger batch, S1/S2 DMA boundaries; Intel 231466-005 pp. 4 and 7; prior T507 D7 claim requires correction for the uncovered secondary first service. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [contracts](../etc/architecture/t539-boundary-contracts.md), [ledger](../etc/evidence/t539-chip-migration-ledger.md), [prior DMA audit](../etc/evidence/t507-s4-dma-controller-reaudit.md). |
| Files And ABI Surface | NXVM dma.c, existing DMA channel regression, NXVM evidence/status/history/proposal/index and eight 0539 EXEs. No new public API or struct; retain original phase handlers. |
| Applicable Rules | Task Reading Set; source policy; Architecture/Coding/Document/Execution; architecture-governance and coding-governance. One execution path, observable failure ordering, original cohesive handlers, bounded completeness and actual-diff acceptance. |
| Verification | Demonstrate regression failure before repair and pass after; cover channels 0-3/5-7, normal/TM, demand/single/block and transfer directions; full NXVM units and default integration x64/x86, each other profile/width boot once; specialized static aggregate, six manifests, documentation gate, PE/version/SHA and unchanged INIs. |
| Expected Markers | Execute is called only by the existing phase handler, never directly by arbitration. Before S4 no provider/RAM/address/count/TC publication; correct single completion thereafter. Existing M2M, cascade, READY and transaction regressions remain. |
| Asset Needs | Read-only existing Intel PDF and existing external profile firmware/media. No import, license change or external asset modification. |
| Reporting Requirements | Report the false blanket phase-order conclusion and the concrete defect, negative-control proof, full regression outcomes, source/test delta and actual-change acceptance. |
| Stop Conditions | Evidence contradicts the selected phase repair, or regressions require unrelated behavioral changes; failing required tests block closure. Never weaken a test or reclassify an incorrect path as compatible L3. |
| Exit Criteria | Entire first-service channel/mode family covered; bypass removed without new state/API; affected receivers and gates pass; complete NXVM delivery pushed then actual-diff accepted. DMA extraction stays pending in the ledger. |
| Original Owner Request | Independent decoupled chips in x86/devices, NXVM board integration only; automatically admit each subsequent S without another manual approval. |
| Similar-Issue Sweep | Inspect every Execute caller, primary/secondary grant, M2M half-cycle, accelerated fixture entry and scheduler transaction/clock route. Cover all seven bindable channels with normal/TM and demand/single/block; preserve callback-before-commit failure rules. Extraction-only page/lane/binding concerns remain in the DMA ledger row, not silently accepted. |

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
were 6cf3cee40; S6's complete delivery now includes the first-service repair and
rebuilt eight artifacts. [S6 evidence](../etc/evidence/t539-s6-dma-first-service.md)
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
