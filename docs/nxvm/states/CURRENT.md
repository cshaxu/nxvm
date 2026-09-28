# Project Status

## Current Work

M5 T539 remains open. S1-S10 are accepted. S11 is automatically admitted under
the owner's 2026-09-28 authorization: repair the pending FDC seek/completion
ownership and bounded-admission class before chip extraction.

| Task | Progress |
| --- | --- |
| T539 S11 | Pending-operation repair and all receivers verified; awaiting coordinator actual-diff acceptance. READY/drive qualification remains a subsequent FDC cutover gate; HDC, video, CPU/FPU and final ledger review remain. |

## Active S11 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S11; one session switches coordinator/executor roles. |
| Admission And Approval | Owner automatic-S authorization; NXVM production/tests/docs and its eight affected artifacts only. Shared, MyNES, INI and siblings remain unchanged. |
| Objective | Remove dependence on the most recently received command for an older seek completion; bound pending completion storage and correctly admit commands around SIS, reset and parallel seeks. |
| Non-goals | No FDC relocation, new public device framework, READY wiring guess, timing-grade upgrade or legacy unready-policy rename. |
| Reference Baseline | S10 P1 7fa0f75d5 and its coordinator acceptance; executable baseline remains S9. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), [S10 decision](../etc/architecture/t539-s10-fdc-boundary.md) and [finite ledger](../etc/evidence/t539-chip-migration-ledger.md). |
| Files And ABI Surface | NXVM fdc.c/h and original FDC test family; touch scheduler/board only if required by the same ownership fix. No Shared ABI change. |
| Applicable Rules | Unique state owner, bounded inputs, unchanged original command style, no second FIFO/state authority, target-specific commits and actual-diff review. Read Architecture/Coding/Execution and source policy plus the selected Intel pages. |
| Verification | Reproduce negative controls without undefined-memory test execution; cover mixed seek/recalibrate across all four units, intervening commands, SIS draining, reset/cancel and excess/repeated requests. Full unit x64/x86, default integration both widths, other profile/width boots once each, static/doc gates; rebuild all eight optimized 0539 EXEs and inspect identities. |
| Expected Markers | Original FDC markers plus explicit completion/admission regression marker. Existing 120-row readiness characterization remains unless a separately admitted semantic correction replaces it. |
| Asset Needs | Existing authorized external profiles/media only; unit inputs remain in-code. No master or INI mutation. |
| Reporting Requirements | Report reproduced causes, source-supported admission behavior and any undocumented hardware case explicitly; count source/test diff and record artifact hashes. No partial P delivery. |
| Stop Conditions | New protected-source/import need or a semantic claim unsupported by inspected evidence. Do not force a BIOS-specific result to make a boot pass. |
| Exit Criteria | No shared-command identity leak, no unbounded completion append, original/negative tests pass, all affected receivers verified once, complete implementation pushed and coordinator actual-diff accepted. |
| Original Owner Request | Independent chip mechanisms in x86/devices, board integration retained in NXVM, no duplicate path, automatic admission of each S. |
| Similar-Issue Sweep | Every writer/reader of pending seek, command buffer, completion cause, result arrays, IRQ and reset; all unit indices and same-unit overlap, not only one failing sequence. |

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
