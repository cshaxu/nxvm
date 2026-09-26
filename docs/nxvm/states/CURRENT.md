# Project Status

## Current Work

M5 T538 remains open. S2 and S3 are accepted and closed; S4 is active.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) own the remaining whole-task repeated boot qualification.

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T538 S4 after accepted S3; no reopening or duplicate closure. |
| Admission And Approval | Owner requests a new S to import reviewed SoftPC repairs and verify complete six-root equality on 2026-09-25. Allowed targets: Shared, NXVM and MyNES receiving artifacts/evidence only. |
| Objective | Import SoftPC b79769c1 Console output-extent and retry-coverage repair unchanged, and prove all six source/test roots match that committed source. |
| Non-goals | No sibling writes, new API, product source or INI/media changes, font scaling, viewport resizing or whole-task boot qualification claim. |
| Reference Baseline | NXVM 763a70134; SoftPC b79769c1 clean worktree, including its reviewed S8 P2 fix. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md), Console output batch. |
| Files And ABI Surface | Six changed files in src/lib and test/lib; other four roots and test/register.cmake verified unchanged. Eight NXVM 0538 and two MyNES 0043 receivers plus target-owned evidence. ABI unchanged. |
| Applicable Rules | EXECUTION, ARCHITECTURE, CODING, DOCUMENT; both product guides, architecture/coding/UI and source policies. Only broker owns native frame storage; cache validity and retained coverage are separate facts; no product policy enters Lib. |
| Verification | Exact six-root path/byte comparison to pinned source and current sibling; all six manifests and component/Types gates through complete suites. NXVM full units x64/x86 and external integration; MyNES full suites both widths; all ten optimized stripped product builds; INI hashes unchanged; both documentation gates and actual-diff review. |
| Expected Markers | Steady 25-row frame writes 25 rows; 50-to-25 clears the old tail; partial writes retain retry coverage; 50-to-30 native shrink with a 25-row frame succeeds; undersized viewport is not a failure. |
| Asset Needs | Existing lawful external assets only; preserve masters, configurations and snapshots. Reuse current build trees for receiver verification and the immediately following T qualification. |
| Reporting Requirements | Report equality per root, pinned source and final sibling state, complete tests, ten receiver hashes, per-target pushed commits and remaining T gate. |
| Stop Conditions | Unreviewed sibling changes, license/API changes, test regressions or unrelated product edits require coordinator review; do not chase a moving upstream silently. |
| Exit Criteria | Reviewed import is byte-identical to pinned six-root corpus; required tests/builds/manifests pass; all receivers and evidence are committed/pushed per target; coordinator reviews actual changes; clean tree. T538 stays open. |
| Original Owner Request | Close current S, admit a new S to import SoftPC fixes, then verify six components are completely identical. S3 was already closed. |
| Similar-Issue Sweep | Reconcile all cache/coverage resets and raw-write extents in Console broker plus native/fake tests; check steady, shrink, stream-write invalidation and failed-write retry. Linux and KVM leaves do not own the Win32 backing rectangle. Existing cooked-history rollback TODO remains separate. |

## Retained Progress

| Task | Progress |
| --- | --- |
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
MyNES 132/132 per width. All ten receivers are rebuilt; target-separated
delivery is being finalized. Whole-task three-fresh-launch qualification
remains pending. The separate cooked-history rollback debt remains in TODO.

## Historical Context

[Archived status and packets](../history/M6-T41-and-prior-status-archive.md)
preserve earlier deliveries and their original hosting context. The five
unrelated [Queue](QUEUE.md) candidates are unchanged.
