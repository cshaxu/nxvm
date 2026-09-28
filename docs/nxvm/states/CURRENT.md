# Project Status

## Current Work

M5 T539 remains open. S5 PIC extraction is closed after actual-change review.
Subsequent bounded batches continue under the owner's automatic-S authorization
dated 2026-09-28; no additional manual admission is required.

| Task | Progress |
| --- | --- |
| T539 S5 | Accepted: Shared d6dc6ca3a and NXVM 6cf3cee40 pushed; actual-change review complete. All bounded exit criteria satisfied; remaining T539 inventory is not accepted by this closure. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S5 after S4 acceptance be86dee2e. |
| Admission And Approval | Owner authorized automatic admission of every S on 2026-09-28. Coordinator admits the complete PIC ledger batch, with Shared and NXVM targets, under that authority. |
| Objective | Extract one opaque Types-only 8259 mechanism, reconnect the XT single controller and PC/AT pair through NXVM-owned routing/aggregation, and remove private chip access and the old implementation. |
| Non-goals | Other chip extraction, new timing qualification, changing BIOS/media/INI, Lib/Common/MyNES edits, sibling writes, new generic device frameworks or weakening existing PIC/CPU scenarios. |
| Reference Baseline | be86dee2e; the pic.c, pic.h and pic_interface.h ledger row; S1 design and S2 interrupt/lifetime contracts. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [contracts](../etc/architecture/t539-boundary-contracts.md), [ledger](../etc/evidence/t539-chip-migration-ledger.md), [S5 boundary review](../etc/architecture/t539-s5-pic-boundary.md). |
| Files And ABI Surface | Shared src/x86/devices/pic8259, matching tests/build/guards/manifests; NXVM pic_bus board adapter, all CPU/device/machine consumers and tests, build/docs and eight 0539 EXEs. Opaque local command/data, IRQ input, cascade signals/selection, acknowledge, reset and timing/deadline APIs; no peer chip pointer. |
| Applicable Rules | Task Reading Set; source policy; Architecture/Coding/Document/Execution; architecture-governance then coding-governance. One chip state owner; board owns ports, source aggregation and pair wiring; Types-only shared source; preserve handler style and behavior; failure cleanup and separate target commits. |
| Verification | Standalone x86 chip suites x64/x86; full NXVM run-unit-tests both widths; run-integration-tests default both widths and each remaining profile/width boot once; verify-current-specialized-gates; six manifests and documentation gate; eight stripped optimized 0539 products with PE/version/SHA and unchanged INIs. |
| Expected Markers | No App/Common/peer-private include in chip; no PIC register structure outside its component; one priority/ICW/OCW implementation; board-only port decode, IRQ-source counts and cascade wiring; retained poll/SFNM/spurious/INTA/reset/deadline regressions. |
| Asset Needs | Existing external profile firmware/media only; existing source/ledger evidence. No external asset import, modification or new reference-derived semantics. |
| Reporting Requirements | Explain boundary findings and any objection before implementation; report verification, source/test delta, artifact identities and actual-diff acceptance. |
| Stop Conditions | Unsupported behavioral change or new source/license requirement outside this batch; a failing receiver blocks closure. Automatic admission does not permit hiding unresolved semantics or unrelated edits. |
| Exit Criteria | Entire PIC ledger batch and caller sweep have a disposition; standalone chip and board reconnection replace old path; required tests/artifacts pass, target-separated deliveries pushed and coordinator actual-change acceptance recorded. T remains open. |
| Original Owner Request | Independent decoupled chips in x86/devices, NXVM board integration only; automatically admit each subsequent S without another manual approval. |
| Similar-Issue Sweep | Search all src/test/cmake for t_pic, PIC private data, cascade peer pointers, IRQ-source aggregation and initialization/acknowledgement/deadline callers; migrate all hits or explicitly retain board-only roles. Tests use register operations/owned fixtures, not a new test-only public state dump. |

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
are 6cf3cee40. [S5 evidence](../etc/evidence/t539-s5-pic-extraction.md) owns the
source mapping, artifact hashes and transaction rollback evidence. Full sibling parity is
not claimed; no sibling repository was modified.

Verification: NXVM 341/341 units and 20/20 default-profile external integration
per width; all six non-default profile/width boot matrices pass once.
Independent chip suites are 12/12 per width; the specialized static aggregate,
extended PIC boundary and six manifests pass. MyNES has no x86 dependency or artifact input change.
The S5 temporary build trees are removed; the two S3 NXVM incremental trees
remain for the immediately next chip batch. Cooked-history rollback debt
remains in [TODO](TODO.md). This bounded regression acceptance does not claim
complete hardware qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification and final accepted single-pass matrix.
