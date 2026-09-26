# Project Status

## Current Work

M5 T538 S2 is completing the owner's accepted 5170 keyboard repair.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) retain the whole eight-pair task.
Owner directed the Console capacity repair to a separate S3 on 2026-09-25;
S3 is authorized but begins only after S2 delivery and acceptance.

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T538 S2, after accepted S1. |
| Admission And Approval | Owner admitted the boot-pair repair and now explicitly accepts the 5170 repair as S2, transferring the separate Console issue to S3. |
| Objective | Close AT intermittent 303 on both host widths at the KBC owner, with unchanged firmware, media and INIs. |
| Non-goals | No Shared or MyNES source delivery in S2; no BIOS workaround, automatic F1, host input flush or fabricated boot success. |
| Reference Baseline | S1 5e6ed82cd / ad99ae0fa, original Shared corpus and eight 0535 products. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md). |
| Files And ABI Surface | NXVM KBC, existing controller regression, deployed observer, version recipe, eight 0538 products and NXVM evidence. No new public API. |
| Applicable Rules | Shared EXECUTION, ARCHITECTURE, CODING, DOCUMENT; NXVM ARCHITECTURE, CODING, UI and source policy. KBC solely owns serial delivery and typematic state. |
| Verification | Before/after Return-release reproducer on both widths; native-byte/inhibit regressions; AT reset/resume and stop/start; full NXVM units both widths; external integration; rebuild all eight optimized 0538 products without the pending Console change. Review all actual changes and documentation gate. |
| Expected Markers | Old code reproduces 303; repaired code reaches the DOS installer on both widths; device regression fails before and passes after. |
| Asset Needs | Frozen S1 external ROM/CMOS/media and four unchanged INIs; overlay masters stay unchanged. |
| Reporting Requirements | Separate accepted KBC proof from the Console defect and final whole-task repeat matrix. Record build/source identity and transferred work explicitly. |
| Stop Conditions | Any unexplained KBC regression, asset change or mixed Shared build input blocks S2 delivery. |
| Exit Criteria | Complete NXVM-only repair and artifacts verified, reviewed, committed and pushed; owner-approved Console transfer documented, not claimed fixed by S2. |
| Original Owner Request | Repair intermittent 5170 303 on x64 and x86; then close its S and admit a separate S for Console buffer behavior. |
| Similar-Issue Sweep | All native-key submissions and automatic repeats share one serial backlog; unmatched breaks cannot start repeats. Review all four profile consumers and preserve XT's distinct keyboard mechanism. |

## Progress

S1 is accepted. S2's KBC reproducer, regression and both-width installer,
reset/resume and stop/start evidence are complete. Model 40 x86's visible
capture and paused text memory agree on the full installer; the earlier partial
hidden capture did not establish a guest hang. The owner accepts this bounded
repair, not whole-task qualification.

The already-developed Console backing-buffer change and its receiver verification
are transferred to S3. Its six Shared files are temporarily isolated during S2
builds; a preserved patch remains in the ignored active diagnostic directory.
S2 binaries use the original Shared implementation. All eight 0538 builds and
architecture checks passed; isolated units pass 335/335 per width and external
integration 20/20. Final AT processes both reach Setup under the 50 ms trigger.
No INI changes occurred. Whole-task three-fresh-launch coverage remains pending.

## Retained Closures

| Task | Closure |
| --- | --- |
| T41 | Owner-accepted closed baseline; Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b. |
| T538 S1 | Accepted baseline inventory: implementation 5e6ed82cd and acceptance ad99ae0fa; no production repair claimed. |

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight verified 0538 EXEs replace the 0535 baseline in the S2 delivery.
The canonical Shared baseline remains 75099c178. MyNES 0043 receiver changes
belong to pending S3, not this NXVM-only delivery.

## Historical Context

[Archived status and packets](../history/M6-T41-and-prior-status-archive.md)
preserve earlier deliveries and their original hosting context. The five
unrelated [Queue](QUEUE.md) candidates are unchanged.
