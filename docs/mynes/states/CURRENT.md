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
| T44 S5 | Active corrective: repair shared-test ownership, Types constant authority, and the named IBM PC owner-local forwarding residue. |

Final audit: [T44 closure](../etc/evidence/m7-t44-s3-closure.md).

## Active T44 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective: T44 is the latest closed numeric MyNES task; S5 is its next unused subtask. |
| Admission And Approval | Owner approved S5 on 2026-10-07 after the `dd68d38cd` static audit, explicitly authorizing the listed shared test-boundary, Types, and IBM PC cleanup. |
| Objective | Remove the confirmed product dependency from Shared IBM PC negative checks, make used integer-bound constants Types-owned, verify media-unload close failure propagation, and remove named IBM PC same-owner forwarding residue without changing guest behavior. |
| Non-goals | No Lib behavior change or Common change; no warning-policy cleanup, new framework, state machine, API, guest behavior change, artifact/config/media/snapshot edit, or broad dead-code claim. The narrow Types header edit is limited to missing integer-bound spellings needed by the audited Shared consumers. |
| Reference Baseline | `dd68d38cd`, clean worktree before S5 admission; static audit names two nested CMake product dependencies, direct integer constants, and three IBM PC local forwarding opportunities. |
| Candidate Proposal | Corrective continuation of [T44 measured efficiency](../history/M7-T44-measured-emulator-efficiency.md); bounded shared quality repair only. |
| Files And ABI Surface | Shared: `src/lib/types/types_interface.h`, `src/x86/`, `src/ibmpc/`, `test/x86/`, `test/ibmpc/` and manifests. NXVM: only the necessary product test relocation/registration beneath `test/app-nxvm/` and its build declaration. No MyNES production change and no Lib behavior/Common change. |
| Applicable Rules | MyNES Architecture/Coding, Shared Execution/Architecture/Coding/Documentation, NXVM test-boundary rules for the relocated product checks, and CONTRIBUTING reading set. |
| Verification | Reproduce each nested CMake dependency before moving it; run each independently selectable shared suite and its manifest; run relocated NXVM checks; perform Types-boundary sweep; build x64/x86 affected consumers and run focused regressions. Run the complete repository-only unit suite before S closure. |
| Expected Markers | `test/ibmpc` contains no NXVM product-checker dependency; every touched integer limit uses Types vocabulary; FDD/HDD unload reports a final Storage close failure while still consuming the lease under the existing Storage contract; public FDD/HDD insert entry owns its implementation directly; normal media and CPU/board behavior remains unchanged. |
| Asset Needs | None; no external ROM, media, firmware, configuration, or snapshot input changes. |
| Reporting Requirements | Report per-item before/after owner and file disposition; distinguish moved product tests from retained component tests; list actual added/removed/net source/test lines and all unverified paths. |
| Stop Conditions | A required correction needs a Lib behavior/Common change, a public Types vocabulary beyond the narrow missing integer limits, guest behavior change, external asset, or another App source change. |
| Exit Criteria | Every confirmed item has an owner-local implementation or an explicit evidence-backed disposition; manifests and boundary sweeps pass; no product checker remains in Shared test roots; all required verification and target-scoped P commits are complete. |
| Original Owner Request | “收口S4，准入S5” followed by the owner-supplied static audit and bounded recommended repair scope. |
| Similar-Issue Sweep | Search all nested Shared `.cmake` checks for `cmake/nxvm`, all Shared source/test integer bound constants, and all IBM PC public one-line forwarding wrappers before closing. |

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
