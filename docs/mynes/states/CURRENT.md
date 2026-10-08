# Project Status

## Current Work

M7 T44 is reopened as a narrow corrective task after the shared-runtime
receiving qualification exposed two repeatable MyNES lifecycle integration
timeouts. S1 palette lookup and S2 selective Mapper simplification remain
qualified in the current 0044 pair.

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
| T44 S4 | Accepted by owner: Core Driver creates a native audio sink only for product composition; repository-only fixtures no longer wait on a physical endpoint. Shared FDD/HDD unload now returns its actual Storage close result. Focused dual-width and all 12 integration routes per width passed; the x86 CPU exhaustive unit was not rerun under the terminal's 30-second command limit and was not claimed as fresh proof. |
| T44 S5 | Reopened corrective: retain the accepted ownership/Types cleanup, then repair the declared IBM PC KVM dependency and add the missing close-failure injection proof. |

Final audit: [T44 closure](../etc/evidence/m7-t44-s3-closure.md).

## Active T44 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective: T44 is the latest closed numeric MyNES task; S5 is its next unused subtask. |
| Admission And Approval | Owner reopened S5 on 2026-10-07 after `7257d068f`, explicitly authorizing the declared IBM PC KVM dependency correction and close-failure injection proof. |
| Objective | Retain the accepted product-test ownership, Types, and forwarding cleanup; add the declared IBM PC Machine-to-KVM dependency and prove that FDD/HDD close failure reports an error after consuming the media lease. |
| Non-goals | No Lib behavior/Common change; no public API, warning-policy cleanup, framework, state machine, guest behavior, artifact/config/media/snapshot edit, or broad dead-code claim. The test-only close-result injector does not exist in production linkage. |
| Reference Baseline | `7257d068f`, clean worktree after S5's accepted ownership/Types/forwarding repair; receiving review found the remaining Machine-to-KVM declaration gap and missing adapter-level close-failure proof. |
| Candidate Proposal | Corrective continuation of [T44 measured efficiency](../history/M7-T44-measured-emulator-efficiency.md); bounded shared quality repair only. |
| Files And ABI Surface | Shared IBM PC: `src/ibmpc/verify_corpus.cmake`, `src/ibmpc/CMakeLists.txt`, `test/ibmpc/` and manifests. The test directly compiles the two media owners with a private result injector. No Lib/Common/public ABI or product source change. |
| Applicable Rules | MyNES Architecture/Coding, Shared Execution/Architecture/Coding/Documentation, NXVM test-boundary rules for the relocated product checks, and CONTRIBUTING reading set. |
| Verification | Run the IBM PC corpus gate before and after the dependency declaration; build and run the new close-failure test x64/x86 with existing media tests; verify manifests; then run the complete repository-only unit suite x64/x86 before S closure. |
| Expected Markers | IBM PC Machine declares and links its existing KVM event dependency; injected final-close failure makes both FDD/HDD removal return `LIB_STATUS_IO_ERROR` while `has_media` is false and the owned medium is null; normal media behavior remains unchanged. |
| Asset Needs | None; no external ROM, media, firmware, configuration, or snapshot input changes. |
| Reporting Requirements | Report per-item before/after owner and file disposition; distinguish moved product tests from retained component tests; list actual added/removed/net source/test lines and all unverified paths. |
| Stop Conditions | A required correction needs a Lib behavior/Common change, a public Types vocabulary beyond the narrow missing integer limits, guest behavior change, external asset, or another App source change. |
| Exit Criteria | Every confirmed item has an owner-local implementation or an explicit evidence-backed disposition; manifests and boundary sweeps pass; no product checker remains in Shared test roots; all required verification and target-scoped P commits are complete. |
| Original Owner Request | “收口S4，准入S5” followed by the owner-supplied static audit and bounded recommended repair scope. |
| Similar-Issue Sweep | Search all IBM PC Machine includes and link targets for KVM-base consistency; inspect every FDD/HDD medium destroy caller for a close-result disposition; retain the earlier nested-CMake, Types-bound, and forwarding sweeps as accepted evidence. |

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
