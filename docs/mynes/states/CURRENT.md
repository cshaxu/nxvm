# Project Status

## Current Work

M7 T44 is closed. Its accepted S1 palette lookup and S2 selective Mapper
simplification remain qualified in the current 0044 pair. Corrective S4/S5
also closed the declared Shared receiving gaps without changing MyNES runtime
behavior.

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
| T44 S5 | Closed after owner acceptance: `src/ibmpc` now declares its existing KVM event dependency, and the owner-local FDD/HDD close-failure injection proof confirms error propagation after media lease consumption. The Shared P `56b81532c` and receiving product artifacts are pushed. |

Final audit: [T44 closure](../etc/evidence/m7-t44-s3-closure.md).

## T44 S5 Closure

Corrective S5 is accepted. The corpus dependency gate passes on x64/x86 after
declaring the existing `ibmpc-machine-conversion -> kvm-base` edge. The new
owner-local FDD/HDD close-failure test passes on both widths and confirms that
each remove operation returns the close error only after clearing the consumed
medium state. No Lib or Common behavior, public ABI, MyNES runtime source,
INI, snapshot, media, or ROM changed. T44 has no active packet.

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
