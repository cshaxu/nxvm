# Project Status

## Current Work

M5 T538 remains open. S1-S5 are accepted and closed; S6 is active.
S5 imported production unchanged and delivered the owner-approved test repair
for SoftPC to import. [S5 evidence](../etc/evidence/t538-s5-input-reset-import.md)
records the return delta and verification.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) retain whole-task repeated boot
qualification.

[S6 evidence](../etc/evidence/t538-s6-final-qualification.md) records the completed
24-run batch awaiting coordinator review: 18 boot terminals, five XT `9C 301`
failures and one unclassified default x86 early pause. T closure is rejected;
passing units/integration do not supersede these deployed failures.

## Active Packet: M5 T538 S6

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; next unused S6 of open T538. |
| Admission And Approval | Owner approved final closure verification on 2026-09-27: “好 你进行这个收口验证”. One session performs coordinator and executor roles sequentially. NXVM-only verification; no Shared or MyNES changes authorized. |
| Objective | Qualify all eight currently deployed EXE/INI pairs with three independent fresh launches each; reconcile the complete T538 ledger and closure predicates. |
| Non-goals | No INI/media edits, synthetic F1, production repair, new hardware qualification, shared import or next queued task. |
| Reference Baseline | Clean c134b0982; unchanged S5 0538 artifacts and recorded input hashes. |
| Candidate Proposal | [T538 proposal](../proposals/m5-deployed-boot-pairs.md); [coverage ledger](../etc/evidence/t538-boot-pairs.md). |
| Files And ABI Surface | NXVM current/history/evidence and proposal disposition only; existing deployed observer and build trees reused. No ABI or executable input change planned. |
| Applicable Rules | Execution: complete batch, unit/integration gates, per-target pushed P and actual-diff review. Document: single current authority and history retention. Source policy: external immutable media/firmware only. Architecture/coding unchanged; no production edit. |
| Verification | Existing deployed observer: each of four profiles on both widths, three sequential 180-second observations, 50ms start Return release; owned-tree watchdog at 195 seconds. Model40 x86 diagnostic extension to 300 seconds only after demonstrated progress. Full NXVM non-desktop and desktop unit suites both widths, established external integration, six manifests, documentation gate and diff check. |
| Expected Markers | Every launch reaches DOS prompt/date entry or installer, not merely a living process; input and pause/resume usable. No 30x error or unclassified exit/hang. |
| Asset Needs | Actual adjacent NXVM.ini and external configured masters; verify hashes before/after. Logs/captures under ignored build/t538-s6, no instruction trace; retain only for this verification and immediate diagnosis. |
| Reporting Requirements | Report batch progress, failures separately from success, final 24-case dispositions, suite totals, unchanged artifact decision and pushed commits. |
| Stop Conditions | An unresolved case blocks T closure. Reconcile complete affected batch before proposing repairs; shared changes or scope expansion require owner review. Stop only owned observer/child processes. |
| Exit Criteria | Complete and reconcile all 24 case dispositions plus applicable tests and governance; actual-change coordinator review, target-scoped commit/push and clean worktree. A completed verification may report a failed T gate; no failed or unclassified run may be called qualified. Close T only if every proposal exit is proved. |
| Original Owner Request | “好 你进行这个收口验证”; verify the remaining repeated deployed boot qualification before claiming T538 closed. |
| Similar-Issue Sweep | Consume all eight deployed pairs across both host widths and all three repetitions, covering shared input/presentation paths; recheck retained S2-S5 regressions through full suites. No new repair class claimed. |

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
qualification now has S6 evidence and is not satisfied. Cooked-history rollback
debt remains in TODO.

## Historical Context

[Task history](../history/M5-T538-deployed-boot-pairs.md) archives the accepted
S5 packet and actual-change review.
[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. The five unrelated
[Queue](QUEUE.md) candidates are unchanged.
