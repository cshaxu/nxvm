# Project Status

## Current Work

M5 T538 remains open. S1-S6 are accepted and closed; S7 is active.
S5 imported production unchanged and delivered the owner-approved test repair
for SoftPC to import. [S5 evidence](../etc/evidence/t538-s5-input-reset-import.md)
records the return delta and verification.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) retain whole-task repeated boot
qualification.

[S6 evidence](../etc/evidence/t538-s6-final-qualification.md) records the completed
24-run verification accepted in 9d5e0ed0a: 18 boot terminals, five XT `9C 301`
failures and one unclassified default x86 early pause. T closure is rejected;
passing units/integration do not supersede these deployed failures.

S7's owner-reviewed Shared orphan-release repair is pushed as 064b9619b.
NXVM INFO now reports session lifecycle instead of worker lifetime.
[Evidence](../etc/evidence/t538-s7-orphan-release.md) records the complete repaired
batch, ten rebuilt receivers and 8/8 successful current EXE/INI pairs. Previous
XT `0E 301` and default early-pause observations remain historical/unexplained
under the owner's explicit acceptance disposition. Delivery/coordinator review
is in progress; no next task is admitted.

The owner requests another complete verification and authorizes closure if it
passes. The owner subsequently reduces qualification to one successful run per
EXE/INI pair and explicitly approves closure after those eight passes. Already
passed pairs are not rerun; each remaining fresh process retains the bounded
180-second observation, unchanged inputs and confirmed pause/Debug handoff.
Historical failures remain recorded; a passing repeated batch is the owner's
conditional acceptance disposition, not a claim that their unobserved causes
have been proven. Any reproduced failure prevents closure.

## Active Packet: M5 T538 S7

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; next unused S7 of open M5 T538. |
| Admission And Approval | Owner requests continued investigation and repair on 2026-09-27. One session performs coordinator and executor roles sequentially. Owner subsequently approves the reviewed Shared orphan-release repair: consume breaks absent from the existing source-local held-key ledger, without replaying pending modifiers. NXVM and MyNES are affected receivers. |
| Objective | Resolve the complete S6 residual batch: XT startup input/POST and pause handoff, and the separate default x86 premature pause. Distinguish observer defects, external interference and product defects before changing their owning mechanism. |
| Non-goals | No BIOS-specific input discard, F1 workaround, INI/media substitution, broad timing downgrade, unreviewed Shared/MyNES changes or next queued task. |
| Reference Baseline | Clean ea250fa56; unchanged S5 0538 receivers and S6 immutable EXE/INI/media identities. All 24 prior dispositions remain evidence, not overwritten by new passes. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md); [convergence ledger](../etc/evidence/t538-boot-pairs.md); [S6 residual batch](../etc/evidence/t538-s6-final-qualification.md). |
| Files And ABI Surface | NXVM integration observer/regressions, INFO lifecycle report, task records and eight dual-width 0538 receivers; Shared kvm-base/hotkey.c, its contract, owner-local keyboard regressions and manifests; MyNES current dual-width 0043 receivers. No public ABI changes or unrelated App edits. |
| Applicable Rules | Execution complete-batch and single-target P rules; architecture single input/state owner and no BIOS workaround; coding flat owner-local repair and Types; documentation single current authority; source policy external immutable firmware/media. Architecture/coding governance skills used. |
| Verification | Bounded real EXE/INI replay, isolated serial native tests, paired start Return release delays and semantic pause acknowledgement. Each diagnostic process at most 180 seconds plus 20 seconds cleanup; owned logs under ignored build/t538-s7, no instruction recording. Full NXVM units both widths and integration, full MyNES suites both widths, six manifests and both documentation gates; all ten optimized stripped receivers rebuilt for the Shared production change. T closure needs one successful launch for each of eight current pairs, as subsequently directed by the owner. |
| Expected Markers | No debugger text sent without confirmed monitor handoff. All eight current pairs reach DOS/installer without F1 and acknowledge pause; historical unexplained failures remain explicit under the owner's single-pass acceptance disposition. |
| Asset Needs | Existing external firmware and media only, unchanged adjacent INIs and overlay masters. Retain S6 raw observations and current build trees for immediate comparison. |
| Reporting Requirements | Report confirmed versus suspected causes, Shared repair proposal before edits, complete affected-batch dispositions, tests/build hashes, source delta, reviewed pushed P and remaining gates. |
| Stop Conditions | Further Shared changes beyond the approved orphan-release mechanism require owner review. Any failure in the revised eight-pair qualification prevents closure. Stop only owned processes. |
| Exit Criteria | All admitted residuals reconciled with repairs or the owner's single-pass acceptance disposition, complete units/integration and eight successful current pairs; affected artifacts/evidence pushed, actual-change review and clean tree. Owner permits T closure after these gates, but no next task. |
| Original Owner Request | Continue investigating and repairing the failures exposed by S6 closure verification. |
| Similar-Issue Sweep | Inspect cooked/raw and Window input sources, unmatched releases, reset/retirement and delivered-key ledger, all pause producers, observer command injection and acknowledgement across XT/AT/Model40/default and both widths. Shared hits are reported for approval, not hidden in an App workaround. |

## Retained Progress

| Task | Progress |
| --- | --- |
| T538 S6 | Verification delivery 9d5e0ed0a accepted; T gate failed: five XT keyboard-error runs, one default early pause, and unproved XT pause handoff. No repair or next S admitted. |
| T538 S5 | Accepted Shared 0c71110b0, NXVM fd006cd5c and MyNES 7f0521ab4. Pinned production and test/x86 match SoftPC 40da7d00; two test roots contain the approved return correction. All ten receivers and complete suites pass. |
| T538 S4 | Accepted Shared 4ca7e6401, NXVM 882e6959a and MyNES 57f0e79e5; all six roots exactly match SoftPC b79769c1; ten receivers and both-width suites verified. |
| T538 S3 | Accepted Shared 6a3f3cb25, NXVM 7d29f409b and MyNES a60dcb906; full buffer repair, ten receivers, both-width suites and manifests verified. |
| T538 S2 | Accepted implementation 1d80f11d8: 5170 KBC repair, x64/x86 335/335 units, external integration 20/20, both-width Setup/reset/stop-start proof, eight isolated 0538 products. Separate Console repair transfers to S3. |
| T538 S1 | Accepted baseline inventory: 5e6ed82cd and acceptance ad99ae0fa; no production repair claimed. |
| T41 | Owner-accepted closed baseline; Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b. |

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized stripped 0538 EXEs are deployed with unchanged owner
INIs. S7 artifact SHA-256 values are in its evidence linked above.

Canonical Shared revision is 064b9619b. Lib source/test add the approved orphan
release repair to 0c71110b0. S5's test-path return differences also still require
SoftPC synchronization, including removal of its cross-owner fixture. Common/x86
production is unchanged; complete six-root parity is not claimed.

S7 verification passes: NXVM 336/336 units plus 21/21 static checks per width,
20/20 external integration; MyNES 132/132 per width. All ten receivers are
rebuilt for target-separated delivery. Owner-revised deployed qualification is
8/8 successful single launches. Cooked-history rollback debt remains in TODO.

## Historical Context

[Task history](../history/M5-T538-deployed-boot-pairs.md) archives the accepted
S5/S6 packets and actual-change reviews.
[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. The five unrelated
[Queue](QUEUE.md) candidates are unchanged.
