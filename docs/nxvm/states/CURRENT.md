# Project Status

## Current Work

M5 T538 S3 is active. S2 is accepted and closed on the owner's instruction.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) own the remaining Console repair
and whole-task repeated boot qualification.

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T538 S3, after accepted S2. |
| Admission And Approval | Owner explicitly requested S2 closure and a separate S for the Console repair on 2026-09-25; complete backing storage with a smaller scrollable viewport is approved. |
| Objective | Remove false Console delivery failure caused solely by an undersized visible viewport while preserving all guest text cells. |
| Non-goals | No font scaling, frame truncation, automatic Window fallback, new API, KBC change, owner INI change or unrelated MyNES source change. |
| Reference Baseline | S2 implementation 1d80f11d8 and acceptance 3922622ea; Shared 75099c178; four frozen INIs. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md). |
| Files And ABI Surface | Shared Console broker, two existing Lib tests, README and two manifests. NXVM eight 0538 and MyNES two 0043 receiving binaries/evidence. Public ABI unchanged. |
| Applicable Rules | Shared EXECUTION, ARCHITECTURE, CODING, DOCUMENT; NXVM and MyNES architecture/coding/UI and source policies. Broker is the sole host buffer owner; copied guest frame is not modified to match viewport. |
| Verification | Full/offscreen frame and smaller/scrolled viewport; rejected/ignored buffer growth and clipped native writes; stale lower-row clearing; existing Console handoff. Full NXVM units both widths and integration; full MyNES receiver suites both widths; all six manifests; optimized stripped receivers and documentation/diff review. |
| Expected Markers | Undersized viewport no longer reports IO_ERROR; every active cell exists in backing storage; genuine storage/output failure still propagates. |
| Asset Needs | Existing lawful external inputs only, no master or INI changes. Existing private diagnostic evidence may be reused when source identity matches. |
| Reporting Requirements | Distinguish implemented candidate from deployed artifact and whole-task boot qualification; report both consumers and retained rollback debt. |
| Stop Conditions | New host policy/API, product-state coupling, unexpected receiver regression or edits outside approved targets require review. |
| Exit Criteria | Approved buffer policy and regressions pass; manifests/docs consistent; affected receivers rebuilt and verified; target-separated complete delivery reviewed and pushed. T-level three-fresh-launch matrix remains a later gate. |
| Original Owner Request | Close the 5170 repair S, then admit a separate S to fix Console buffering without shrinking fonts or discarding offscreen data. |
| Similar-Issue Sweep | Inspect raw text delivery, palette-before-geometry, tall-to-short clearing and cooked restoration. The separately recorded partial-startup history rollback remains TODO, not claimed fixed. |

## Retained Progress

| Task | Progress |
| --- | --- |
| T538 S2 | Accepted implementation 1d80f11d8: 5170 KBC repair, x64/x86 335/335 units, external integration 20/20, both-width Setup/reset/stop-start proof, eight isolated 0538 products. Separate Console repair transfers to S3. |
| T538 S1 | Accepted baseline inventory: 5e6ed82cd and acceptance ad99ae0fa; no production repair claimed. |
| T41 | Owner-accepted closed baseline; Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b. |

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. The eight optimized stripped 0538 EXEs replace 0535, with unchanged
owner INIs. S2 source/artifact SHA-256 values remain historical in the ledger.
The canonical Shared baseline is 6a3f3cb25; receiving artifact hashes are in
[S3 evidence](../etc/evidence/t538-s3-console-buffer.md).

S3 verification passes: NXVM 335/335 units per width and 20/20 integration;
MyNES 132/132 per width. All ten receivers are rebuilt. S3 target-separated
delivery is in progress; whole-task three-fresh-launch qualification remains
pending. The separate cooked-history rollback debt remains in TODO.

## Historical Context

[Archived status and packets](../history/M6-T41-and-prior-status-archive.md)
preserve earlier deliveries and their original hosting context. The five
unrelated [Queue](QUEUE.md) candidates are unchanged.
