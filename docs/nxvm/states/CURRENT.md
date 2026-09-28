# Project Status

## Current Work

M5 T539 remains open. S1-S11 are accepted. S12 is automatically admitted under
the owner's 2026-09-28 authorization: qualify and repair FDC drive-input and
status semantics before chip extraction.

| Task | Progress |
| --- | --- |
| T539 S12 | Admitted: FDC READY/Track0/status qualification and repair. Chip relocation remains gated by this contract; HDC, video, CPU/FPU and final ledger review remain. |

## Active S12 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S12; one session switches coordinator/executor roles. |
| Admission And Approval | Owner automatic-S authorization; NXVM production/tests/docs and its eight affected artifacts only. Shared, MyNES, INI and siblings remain unchanged. |
| Objective | Establish one source-qualified drive-input interpretation across data, seek/recalibrate and sense commands; repair demonstrated status/SIS contradictions and resolve the existing unready policy before extraction. |
| Non-goals | No FDC relocation, new public device framework, guessed electrical wiring, timing-grade upgrade, BIOS-specific status patch or renamed compatibility switch. |
| Reference Baseline | S11 P1 a714e194b and its coordinator acceptance; S10 characterization remains baseline evidence, not the hardware oracle. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), [S10 decision](../etc/architecture/t539-s10-fdc-boundary.md) and [finite ledger](../etc/evidence/t539-chip-migration-ledger.md). |
| Files And ABI Surface | NXVM FDC, drive/media binding and board/profile wiring only where required by the same contract; associated tests, gates, evidence and eight artifacts. No Shared ABI change. |
| Applicable Rules | Unique state owner, bounded inputs, unchanged original command style, no second FIFO/state authority, target-specific commits and actual-diff review. Read Architecture/Coding/Execution and source policy plus the selected Intel pages. |
| Verification | Render Intel status/command tables before using OCR bit definitions; compare available reference implementations without copying them. Test READY and Track0 independently from media/motor/select, input loss, parallel operations, no-pending SIS, SCAN result bits, reset/IRQ/DMA cancellation. Full unit x64/x86, default integration both widths, other profile/width boots once each, static/doc gates; rebuild all affected eight optimized 0539 EXEs and inspect identities. |
| Expected Markers | Original FDC and S11 ownership markers plus source-qualified drive/status regressions; replace conflicting characterization expectations with explicit evidence, not silently remove cases. |
| Asset Needs | Existing authorized external profiles/media only; unit inputs remain in-code. No master or INI mutation. |
| Reporting Requirements | Record every changed semantic expectation and source/page; distinguish physical drive position from controller PCN and external pins from chip state. Document unsupported hardware cases, production/test diff and artifact hashes. No partial P delivery. |
| Stop Conditions | New protected-source/import need or a semantic claim unsupported by inspected evidence. Do not force a BIOS-specific result to make a boot pass. |
| Exit Criteria | Drive-input/status contract and all demonstrated contradictions in this packet resolved without a machine-name chip branch; negative and receiver tests pass, complete implementation pushed and coordinator actual-diff accepted. An unproven board contract requires explicit coordinator re-plan, not fabricated readiness or a false extraction claim. |
| Original Owner Request | Independent chip mechanisms in x86/devices, board integration retained in NXVM, no duplicate path, automatic admission of each S. |
| Similar-Issue Sweep | Every defined FDC command, READY/media-change sampling, Track0/PCN, status masks, SIS/reset causes, mid-transfer input loss and all four unit indices; retain S11 pending-operation bounds. |

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
Current NXVM source/artifacts are a714e194b. [S11 evidence](../etc/evidence/t539-s11-fdc-seek-ownership.md)
owns pending-operation repair, negative controls, verification and artifact hashes.

Coordinator-role review accepts S11's actual NXVM diff: per-operation identity,
four-slot admission proof, no unsafe negative-test execution, SIS/reset and
parallel-operation regressions, source-qualified versus emulator-only claims,
static gate and all receiver artifacts. No item remains in the bounded S11
packet; its packet is removed. P1 is pushed to origin/master. T539 is not closed.

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
