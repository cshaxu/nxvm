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
| T44 S4 | Active corrective: locate and repair the repeatable lifecycle/native-window completion-event timeouts exposed by current Shared receiving qualification. S1-S3 remain accepted. |

Final audit: [T44 closure](../etc/evidence/m7-t44-s3-closure.md).

## Active T44 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective: T44 is the latest closed numeric MyNES task; S4 is its next unused subtask. |
| Admission And Approval | Owner approved a dedicated next S on 2026-10-07 after x64 serial reproduction of `mynes.integration.lifecycle-smoke` and `mynes.integration.native-window-smoke`; the owner then explicitly added the named Shared IBM PC FDD/HDD close-status propagation. |
| Objective | Repair the owner-local mechanism that prevents MyNES reset/runtime completion events from reaching its integration fixtures, and make IBM PC removable-media unload return its actual Storage close result. |
| Non-goals | No speculative timeout increase, polling loop, Shared API expansion, NXVM change, artifact/config/media/snapshot edit, or attribution of the failure to Audio without direct evidence. |
| Reference Baseline | Current 0044 receiving artifacts and the repeatable x64 serial failures: lifecycle reset completion at `lifecycle_smoke.c:22`; native-window runtime completion at `native_window_smoke.c:218`. |
| Candidate Proposal | Corrective continuation of [T44 measured efficiency](../history/M7-T44-measured-emulator-efficiency.md); this packet owns the bounded receiving regression, not a new performance claim. |
| Files And ABI Surface | MyNES: `src/app-mynes/`, `test/app-mynes/`, and rebuilt MyNES artifacts. Owner-approved Shared exception: `src/ibmpc/machine/media/fdd.c`, `src/ibmpc/machine/media/hdd.c`, their manifest, and their owner-local regressions if an existing seam can prove the close-failure path. No Lib/Common/x86 or NXVM source change. |
| Applicable Rules | MyNES Architecture/Coding, Shared Execution/Architecture/Coding/Documentation, and CONTRIBUTING reading set. |
| Verification | Reproduce serially on x64; trace reset/request/completion ownership; run focused regression on x64/x86. If code changes, rebuild affected 0044 x64/x86 artifacts, run complete repository-only unit suites and relevant external integration routes. Verify FDD/HDD normal unload on x64/x86 and inspect the propagated close-status path. |
| Expected Markers | Reset completion is emitted once for its generation, runtime notification reaches the fixture, no stale completion satisfies a later request, native Window presentation remains functional, and an FDD/HDD unload no longer reports success after Storage close failure. |
| Asset Needs | Existing code-owned fixture only; no external asset change. |
| Reporting Requirements | Report the causal chain before editing production code; list actual changed paths and added/removed/net source/test lines; separately report Shared and MyNES verification. |
| Stop Conditions | A repair requires changing Shared ownership/API, timeout policy, external assets, or another App; stop and obtain explicit owner direction. The named IBM PC status propagation is already approved. |
| Exit Criteria | Root cause is evidenced; any owner-local repair has focused dual-width proof, the named Shared close-status propagation has normal-path proof and a documented failure-path seam disposition, complete required suite result and pushed target-scoped P; no timeout increase is used as a substitute. |
| Original Owner Request | “可以准入下一 S 专门定位这两个 MyNES 生命周期/运行时事件失败。” |
| Similar-Issue Sweep | Inspect all MyNES reset-completion and Common runtime-state fixture sinks, then disposition every same callback/generation path. |

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
