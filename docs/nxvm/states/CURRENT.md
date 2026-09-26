# Project Status

## Current Work

M5 T538 remains open. S2, S3 and S4 are accepted and closed; no S is active.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) own the remaining whole-task repeated boot qualification.

## Retained Progress

| Task | Progress |
| --- | --- |
| T538 S4 | Accepted Shared 4ca7e6401, NXVM 882e6959a and MyNES 57f0e79e5; all six roots exactly match SoftPC b79769c1; ten receivers and both-width suites verified. |
| T538 S3 | Accepted Shared 6a3f3cb25, NXVM 7d29f409b and MyNES a60dcb906; full buffer repair, ten receivers, both-width suites and manifests verified. |
| T538 S2 | Accepted implementation 1d80f11d8: 5170 KBC repair, x64/x86 335/335 units, external integration 20/20, both-width Setup/reset/stop-start proof, eight isolated 0538 products. Separate Console repair transfers to S3. |
| T538 S1 | Accepted baseline inventory: 5e6ed82cd and acceptance ad99ae0fa; no production repair claimed. |
| T41 | Owner-accepted closed baseline; Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b. |

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. The eight optimized stripped 0538 EXEs replace 0535, with unchanged
owner INIs. S2 source/artifact SHA-256 values remain historical in the ledger.
The canonical Shared baseline is 4ca7e6401, identical to SoftPC b79769c1 across
all six roots; receiving hashes are in
[S4 evidence](../etc/evidence/t538-s4-console-import.md).

S4 verification passes: NXVM 335/335 units per width and 20/20 integration;
MyNES 132/132 per width. All ten receivers are rebuilt and pushed in target-separated commits. Whole-task three-fresh-launch qualification
remains pending. The separate cooked-history rollback debt remains in TODO.

## Historical Context

[Archived status and packets](../history/M6-T41-and-prior-status-archive.md)
preserve earlier deliveries and their original hosting context. The five
unrelated [Queue](QUEUE.md) candidates are unchanged.
