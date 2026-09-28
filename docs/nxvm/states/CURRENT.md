# Project Status

## Current Work

M5 T539 remains open. S1-S9 are accepted. S10 is automatically admitted under
the owner's 2026-09-28 authorization: resolve the FDC extraction prerequisite
with a complete readiness/command characterization and concrete owner contract.

| Task | Progress |
| --- | --- |
| T539 S10 | FDC prerequisite is implemented and verified, awaiting coordinator actual-diff acceptance. Source cutover remains gated by the recorded repair obligations. HDC, video, CPU/FPU and final finite-ledger review remain. |

## Active S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S10; single session switches coordinator/executor roles. |
| Admission And Approval | Owner automatic-S approval of 2026-09-28 covers this explicit T539 prerequisite. NXVM documentation/tests/build registration only; Shared, MyNES and sibling source remain read-only. |
| Objective | Resolve the finite ledger's FDC ownership gate before source relocation: inventory every readiness consumer, command/result/IRQ/DRQ/seek/reset interaction, characterize the two current unready policies and specify a neutral chip/drive/board contract with no renamed BIOS-policy flag. |
| Non-goals | No production cutover, new controller qualification, timing upgrade, protected asset import, INI change or source-derived hardware assumption. Test characterization is not hardware correctness. |
| Reference Baseline | Clean pushed 02931b886; S9 source/artifacts remain the executable baseline. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), [contracts](../etc/architecture/t539-boundary-contracts.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md); FDC row remains pending migration. |
| Files And ABI Surface | NXVM FDC contract/evidence and original unit-test family; inspect fdc.c/h, controller/media interfaces, machine construction/scheduler, all board configurations and consumers. No public ABI change in this prerequisite S. |
| Applicable Rules | Architecture unique state/output/publication owner and neutral interfaces; Coding retain table/command semantics and no forwarding framework; Execution finite class coverage, no partial P and actual-diff review; Documentation current/evidence separation; source policy permits read-only references, not copied external code or firmware. |
| Verification | Read complete affected source and historical source-qualified evidence; verify local original manual/reference identities before relying on them. Add table-driven repository-only characterization across all implemented command families and readiness inputs. Run full units on x64/x86 if tests change, documentation gate, links and diff checks. Existing EXEs need no rebuild absent production/build-input change. |
| Expected Markers | Complete finite command/readiness matrix with expected results, IRQ/DRQ, deadlines and side effects; original FDC markers retained. Each uncertainty or defect has an explicit source-cutover disposition, not a generic compatibility switch. |
| Asset Needs | Existing authorized archived controller manual and local read-only emulator references only. Unit tests use in-code media, no external ROM/media. |
| Reporting Requirements | Report decisive contradictions before implementation; durable record maps each FDC state/member and caller to chip/drive/board ownership, all matrix rows and subsequent repair obligations. Commit/push only the complete prerequisite result, then coordinator review and closure. |
| Stop Conditions | New license, protected-copy or unsupported hardware assertion outside this contract. A genuine evidence gap is named, not hidden as L3 or converted into a BIOS workaround. |
| Exit Criteria | All readiness consumers and command families have explicit dispositions; future interface/lifetime/time/failure contract is concrete, old-policy disposition is justified, characterization and required gates pass, actual diff accepted and pushed. FDC migration itself remains unaccepted until its later cutover S. |
| Original Owner Request | Extract all independent chip mechanisms into x86/devices, retain board composition in NXVM, preserve real functionality and remove duplicate paths; automatically admit each S. |
| Similar-Issue Sweep | Reconcile fixed READY, mechanical presence/position, media availability/generation, DOR gates, all read/write/scan/format/seek/reset paths, interrupted commands and pending completions; classify every hit before designing the replacement. |

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
