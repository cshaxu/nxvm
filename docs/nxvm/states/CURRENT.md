# Project Status

## Current Work

M5 T538 remains open. S1-S5 are accepted and closed; no subtask is active.
S5 imported production unchanged and delivered the owner-approved test repair
for SoftPC to import. [S5 evidence](../etc/evidence/t538-s5-input-reset-import.md)
records the return delta and verification.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) retain whole-task repeated boot
qualification; the next S requires admission.

## Retained Progress

| Task | Progress |
| --- | --- |
| T538 S5 | Accepted Shared 0c71110b0, NXVM fd006cd5c and MyNES 7f0521ab4. Pinned production and test/x86 match SoftPC 40da7d00; two test roots contain the approved return correction. All ten receivers and complete suites pass. |
| T538 S4 | Accepted Shared 4ca7e6401, NXVM 882e6959a and MyNES 57f0e79e5; all six roots exactly match SoftPC b79769c1; ten receivers and both-width suites verified. |
| T538 S3 | Accepted Shared 6a3f3cb25, NXVM 7d29f409b and MyNES a60dcb906; full buffer repair, ten receivers, both-width suites and manifests verified. |
| T538 S2 | Accepted implementation 1d80f11d8: 5170 KBC repair, x64/x86 335/335 units, external integration 20/20, both-width Setup/reset/stop-start proof, eight isolated 0538 products. Separate Console repair transfers to S3. |
| T538 S1 | Accepted baseline inventory: 5e6ed82cd and acceptance ad99ae0fa; no production repair claimed. |
| T41 | Owner-accepted closed baseline; Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b. |

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized stripped 0538 EXEs are deployed with unchanged owner
INIs. S5 artifact SHA-256 values are in the evidence linked above.

Canonical Shared revision is 0c71110b0. Production src/lib, src/common, src/x86
and test/x86 match SoftPC 40da7d00 (also unchanged in clean 19e853c0).
The eight test-path return differences require SoftPC synchronization, including
removal of its cross-owner fixture; complete six-root parity is not claimed.

S5 verification passes: NXVM 335/335 units plus 21/21 static checks per width,
20/20 external integration; MyNES 132/132 per width. All ten receivers are
rebuilt and pushed in target-separated commits. Whole-task three-fresh-launch
qualification remains pending. Cooked-history rollback debt remains in TODO.

## Historical Context

[Task history](../history/M5-T538-deployed-boot-pairs.md) archives the accepted
S5 packet and actual-change review.
[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. The five unrelated
[Queue](QUEUE.md) candidates are unchanged.
