# Project Status

## Current Work

M6 T43 and M6 Td S3 remain closed. M7 T44 S1 is accepted after actual-diff
review of Shared P1 ae15f9129 and MyNES P2 b2c973f69; owner accepts S1.
Owner admits M7 T44 S2 on 2026-10-06, first auditing the reasonableness of
Mapper address-computation optimization. Owner approves the audited design and
implementation, including the restored CNROM bank bounds gap.

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
| T44 S2 | Executor complete: selective Mapper simplification and CNROM snapshot repair, both full suites 57/57, exact paired identities and updated 0044 pair. Coordinator pushed-diff review pending; not closed. |

## Active T44 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M7 T44 S2, immediately after accepted S1; T44 remains open. |
| Admission And Approval | Owner accepts S1 and admits S2 on 2026-10-06, then approves the reported selective Mapper design and says to begin implementation. MyNES cartridge/snapshot production, owned tests/tools/docs and paired 0044 artifacts are admitted, including CNROM restored-bank bounds repair. Shared, NXVM and siblings remain excluded. |
| Objective | Audit and bound Mapper address-computation optimization; subsequently adopt only evidence-backed simplification with unchanged mapping, bus events and snapshot continuation. |
| Non-goals | Generic mapper/cache framework, lazy dirty flags, decoded tile caches, suppressed PPU reads/A12 events, new host/runtime/public APIs, changed reset semantics, snapshot version change, Shared/NXVM/INI/ROM/snapshot edits. |
| Reference Baseline | ac5861d90; accepted S1 0044 pair and both fixed-processor probe caches. Measure against S1, not the original pre-palette driver. |
| Candidate Proposal | [Performance proposal](../proposals/emulator-performance-nesticle-study.md), S2; [task ledger](../history/M7-T44-measured-emulator-efficiency.md); [S2 design audit](../etc/evidence/m7-t44-s2-mapper-design.md). |
| Files And ABI Surface | Proposed MyNES core/cartridge.c and private cartridge.h, snapshot reconstruction/validation in snapshot.c, owner unit tests and existing integration benchmark. No public interface change. Up to four PRG and eight CHR lib_u32 offsets (48 bytes per cartridge) is a bounded candidate, not an accepted implementation. |
| Applicable Rules | MyNES Architecture/Coding, shared Execution/Architecture/Coding/Documentation, source policy and existing CPU/bus/cartridge contract. Cartridge alone owns registers and mapping; derived offsets cannot become serialized or independently mutable truth. Use architecture-governance then coding-governance. |
| Verification | Freeze S1 probes; compare Mapper reads over all admitted capacity axes, register/mode/window boundaries, CHR RAM, reset/restore and malformed snapshots. Run full MyNES units/integrations once per width. Compare six-ROM graphics/text fixed-processor identities; pair variants per workload. Where wall time is noisy, record whole-leaf process CPU consumption separately, including warmup/checksums and excluding descheduled time; do not relabel it as pure Core or native FPS. |
| Expected Markers | NROM remains simple; MMC1/UxROM avoid unnecessary persistent caches; CNROM modulo semantics are not replaced with an invalid mask; MMC3 read becomes offset plus local address while IRQ/A12 order remains untouched. |
| Asset Needs | Existing six external ROMs read-only, as in S1; unit fixtures remain code-owned. Keep bounded reference binaries/CSV/warm caches under ignored build for this task only. |
| Reporting Requirements | Report this audit before production edits, including snapshot gap, owner/lifetime/memory and per-mapper disposition; later report actual code delta, repeatable Core/whole-frame benefit or rejection, correctness and current paired 0044 artifact hashes. |
| Stop Conditions | Stop a candidate on mapping/event/state divergence, missing rebuild boundary, snapshot incompatibility, unjustified scope/API/cache growth or nonrepeatable whole-frame benefit; report rather than add a fallback path. |
| Exit Criteria | Owner-reviewed design checkpoint, complete mapper batch disposition, adopted candidate or measured rejection, required correctness/performance proof, actual-diff review and pushed complete delivery. If executable inputs change, rebuild both 0044 products; T stays open. |
| Original Owner Request | Accept S1 and admit the next S, first audit its reasonableness; improve emulator efficiency while keeping the result flat and minimal. |
| Similar-Issue Sweep | All MyNES mapper address formulas, register mutations, construction/reset and snapshot staging/commit; cover NROM/MMC1/UxROM/CNROM/MMC3. Every candidate has a bounded disposition; every discovered unsafe restored mapping is reported before implementation. |

S1 evidence: [palette baseline and qualification](../etc/evidence/m7-t44-s1-palette-performance.md).
Task ledger: [T44 measured efficiency](../history/M7-T44-measured-emulator-efficiency.md).

Archived proposal: [six-component Types boundary audit](../history/M6-T43-six-component-types-boundary-audit-proposal.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).
Current audit: [S13 stopped startup](../etc/evidence/m6-t43-s13-stopped-startup.md).
Artifact evidence: [S10 registration and byte boolean](../etc/evidence/m6-t43-s10-registration-byte-bool.md).

## Current Technical Baseline

- MyNES: optimized stripped 0044 x64/x86 pair in `assets/mynes/`, built from
  S1 implementation b2c973f69 and identified by the hashes in
  [S1 evidence](../etc/evidence/m7-t44-s1-palette-performance.md).
  Full MyNES suites pass 56/56 per width; exact paired game/frame/PCM/state
  proof covers six ROMs and both output modes. Only frame-local palette lookup
  changed production behavior; owner INI and v3 snapshot remain untouched.
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
