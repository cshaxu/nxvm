# Project Status

## Current Work

M6 T43 and M6 Td S3 remain closed. M7 T44 S1 is accepted after actual-diff
review of its pushed implementation; owner accepts S1. S2 is accepted after
actual-diff review of MyNES P1 b22a3bb89. Owner accepts S2 and requests T closure
on 2026-10-06; S3 is admitted solely for final reconciliation and archival.

Latest governance: owner approved latest-pair-only retention on 2026-09-25.
Shared P1 330c8cd18 updates Execution; NXVM P2 b85f72179 removes sixteen old
0533/0534 EXEs; MyNES P3 removes four old 0041/0042 EXEs and updates its asset
guide. At that cleanup the inventory was ten EXEs (MyNES two 0043, NXVM eight 0535); all ten
SHA-256 values match the pre-cleanup baseline. Both documentation gates and
diff/check pass. Configurations, snapshot, media and executable inputs are
unchanged; no rebuild is needed. Deleted EXEs remain recoverable in Git history.

| Task | Progress |
| --- | --- |
| T43 | Closed on owner instruction: S1-S13 accepted batches reconciled in history; final x64/x86 suites 132/132, six-ROM graphics/text matrix 24/24; remaining deferred contracts have explicit TODO receivers. |
| T44 S3 | S1/S2 accepted by owner. Final evidence, scope, code quality, artifact identities and documentation closure audit underway; no runnable changes. |

## Active T44 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M7 T44 S3 after accepted S2; final planned reconciliation, not another optimization task. |
| Admission And Approval | Owner accepts S2 and asks to close T44 on 2026-10-06. MyNES documentation/evidence/archive and owned ignored-output cleanup only. No source, test, tool, configuration, media, artifact or Shared/NXVM changes. |
| Objective | Reconcile every admitted candidate and finite preservation proof, then close T44 only if its actual exits hold. |
| Non-goals | New optimization, new timing claims, new debt scope, hardware completeness claim, source change, repeat unchanged benchmarks/tests or manufacture a binary revision. |
| Reference Baseline | Accepted S2 implementation b22a3bb89 and governance 32f7ed0f8; current optimized stripped 0044 pair with published hashes. |
| Candidate Proposal | [Archived performance proposal](../history/M7-T44-measured-emulator-efficiency-proposal.md); [T44 convergence ledger](../history/M7-T44-measured-emulator-efficiency.md), final S3 batch. |
| Files And ABI Surface | MyNES Current/Queue/history/proposal/evidence index and final audit. Archive proposal with git mv; no ABI or runnable input change. |
| Applicable Rules | Execution task-scale convergence, P lifecycle, scope/artifact retention and actual-diff review; Documentation authority/links; MyNES M7 Roadmap exits and architecture/coding invariants reviewed against accepted production diffs. |
| Verification | Compare all runnable inputs to accepted S2, verify both EXE/INI and six external ROM hashes, inspect final source/test diffs and existing complete 57/57-per-width plus paired 24-route evidence. Reuse those exact-input results rather than rerun; run documentation/link/diff checks on closure edits. |
| Expected Markers | Palette and selective Mapper candidates accepted; IRQ/PPU condition cleanup not admitted; native/per-device timing explicitly unmeasured, no FPS or compounded gain claim; existing unrelated TODO remains visible. |
| Asset Needs | Read-only deployed pair, INI, existing external input identities and ignored bounded measurement records. Retain warm build caches for reuse; dispose expired owned probe outputs only after recording needed summaries. |
| Reporting Requirements | Report any exit gap before closure; otherwise deliver pushed closure, final artifact identity, reused exact-input verification and a clean tree. |
| Stop Conditions | Changed accepted inputs, hash mismatch, unaccounted candidate/defect, broken reference or missing proof blocks T closure; do not silently narrow a required claim. |
| Exit Criteria | Every ledger batch has a direct disposition; task audit and archive complete, implementation P pushed and reviewed, pure governance P closes S3/T44. No new EXE needed for this documentation-only step. |
| Original Owner Request | Owner accepts S2 and asks whether the performance task can now close. |
| Similar-Issue Sweep | Entire T44 changed surface and candidate class; check duplicate production routes, persistent derived-state ownership, snapshot boundaries, protected inputs, scope leakage, stale active links and honest performance attribution. |

S2 evidence: [Mapper qualification](../etc/evidence/m7-t44-s2-mapper-performance.md).

S1 evidence: [palette baseline and qualification](../etc/evidence/m7-t44-s1-palette-performance.md).
Task ledger: [T44 measured efficiency](../history/M7-T44-measured-emulator-efficiency.md).

Archived proposal: [six-component Types boundary audit](../history/M6-T43-six-component-types-boundary-audit-proposal.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).
Current audit: [S13 stopped startup](../etc/evidence/m6-t43-s13-stopped-startup.md).
Artifact evidence: [S10 registration and byte boolean](../etc/evidence/m6-t43-s10-registration-byte-bool.md).

## Current Technical Baseline

- MyNES: optimized stripped 0044 x64/x86 pair in `assets/mynes/`, built from
  S2 implementation b22a3bb89 and identified by the hashes in
  [S2 evidence](../etc/evidence/m7-t44-s2-mapper-performance.md).
  Full MyNES suites pass 57/57 per width; paired frame/PCM/state proof covers
  six ROMs and both output modes. Frame-local palette lookup and selective
  Mapper address simplification are accepted; unsafe restored CNROM banks
  reject without mutation. Owner INI and v3 snapshot remain untouched.
  Superseded 0043 binaries remain in Git history, not the deployment directory.
- Shared: 268464d49; executable behavior is 064b9619b (T538 S7 P1), with
  P4's owner-requested key-specific comment clarification and updated manifest.
  Lib source/test roots add that correction; S5's two test roots retain the
  boundary/retirement corrections for SoftPC to import, including removal of
  its cross-owner fixture. No complete SoftPC parity is claimed. Transfer complete
  roots with removals; test/register.cmake remains unchanged.
  Six manifests and Types gates pass. All receiving Apps were rebuilt together.
- Snapshot `MNS1` guest state remains Core-owned; transient driver stop/frame
  publication state is reset only after successful restore. Deferred work remains
  explicitly listed in [TODO](TODO.md), not implicitly claimed complete.
