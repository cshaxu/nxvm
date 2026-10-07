# Project Status

## Current Work

M6 T43 and M6 Td S3 remain closed. Owner admits M7 T44 S1 on 2026-10-06:
establish the current Windows performance baseline and optimize only the
frame-local palette lookup, as additionally approved on 2026-10-06.

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
| T44 S1 | Palette candidate implemented; x64/x86 exact-output regression and full MyNES suites pass 56/56 each. Fixed-processor before/after measurement and delivery review underway; not closed. |

## Active T44 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M7 T44 S1 after closed MyNES T43; next MyNES numeric T and S1. NXVM T546 remains independently open and unchanged. |
| Admission And Approval | Owner admits baseline and frame-local palette optimization on 2026-10-06. MyNES source/test/build/tools/docs/artifacts are admitted. Owner separately approves only the two root CMake build-preset target updates from 0043 to 0044, delivered as a Shared P; their sole affected consumer is MyNES and both presets are checked. No Shared production, sibling or NXVM edit. |
| Objective | Freeze pre-change Windows x64/x86 baselines, optimize frame-local sample-to-palette lookup, and prove identical output plus measured before/after benefit. Owner additionally admits this implementation in S1 on 2026-10-06; S1 has never closed or committed. |
| Non-goals | Mapper/IRQ/PPU optimization, copying or transliterating Nesticle, approximate/skip guest cycles/bus/IRQ/audio/frames, a new executor/renderer or permanent production profiling overhead, Shared/NXVM/owner INI/ROM/snapshot changes. |
| Reference Baseline | fc0d31623, existing 0043 MyNES pair and owner assets; record exact source/toolchain/flags/hash before measuring. Source/product input and performance-version identity are distinct. |
| Candidate Proposal | [Performance proposal](../proposals/emulator-performance-nesticle-study.md), first gate; [full task ledger](../history/M7-T44-measured-emulator-efficiency.md), S1. |
| Files And ABI Surface | MyNES driver.c palette loop, owner-local regression/benchmark/tool/build registration and evidence; existing production APIs and Lib clocks only. Fixed 512-entry frame-local lookup, no persistent cache or new API. Qualify paired 0044 artifacts for the changed production inputs; do not modify owner configuration. |
| Applicable Rules | Source/research provenance, one state/time/output owner, actual source style and Lib Types, code-owned units and external-only integration, bounded probes and cleanup, target-separated Ps and actual-diff dual-role review. |
| Verification | Fixed synthetic and six supplied game workloads, graphics/text, 60 warmup plus 120 measured guest frames, five diagnostic batches, monotonic counter and both optimized widths. Fix the same logical processor for paired game measurements. Compare complete Core plus conversion intervals and output/state/audio/cycle identities; native presentation and individual CPU/PPU/APU costs are unmeasured, not claimed. Run full MyNES units and integrations once per width, documentation/diff gates, PE/stripped checks and preset/artifact identity. |
| Expected Markers | Baseline ranks real costs; guest and displayed frames are not conflated; already-precomputed RGB is not reimplemented; palette lookup and PPU caches require exact output/bus proof before adoption. |
| Asset Needs | Six existing owner ROMs and snapshot remain external/read-only; code-owned unit scenes have no external file input. Keep bounded results/caches under ignored build; no protected pixels/ROM bytes/local paths or raw traces in commits. |
| Reporting Requirements | Report baseline build/input identity, per-frame distributions and attribution limits, candidate ranking, memory/lifetime constraints, reference rights and actual code delta; no universal/Nesticle-equivalent claim or guessed stage percentage. |
| Stop Conditions | Divergence, unbounded benchmark/cache lifetime, unsupported license, new Shared/runtime API need or an unmeasurable required stage needs explicit disposition; preserve failures rather than weaken checks. |
| Exit Criteria | Complete source/reference/baseline batch with direct reproducibility and qualification proof, fixed candidate decisions, clean owned delivery and pushed P accepted by coordinator actual-diff review. T stays open for subsequent optimizations. |
| Original Owner Request | Use the Nesticle performance ideas to establish MyNES's current baseline and improve emulator efficiency even though Windows is already smooth. |
| Similar-Issue Sweep | All Core/driver execution, PPU/mapper/APU paths, graphics/text adaptation, cached derivations and reset/snapshot invalidation; source/test/build owners and existing paced/native routes. Every claimed hot path or speedup has actual evidence. |

Archived proposal: [six-component Types boundary audit](../history/M6-T43-six-component-types-boundary-audit-proposal.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).
Current audit: [S13 stopped startup](../etc/evidence/m6-t43-s13-stopped-startup.md).
Artifact evidence: [S10 registration and byte boolean](../etc/evidence/m6-t43-s10-registration-byte-bool.md).

## Current Technical Baseline

- MyNES: optimized stripped 0043 x64/x86 artifacts in `assets/mynes/`,
  rebuilt as receivers of NXVM-hosted M5 T538 S7; full suites pass 132/132 per
  width. [Receiver evidence](../etc/evidence/t538-s7-orphan-release-receiver.md) records
  hashes. No MyNES source or configuration changes; T43 remains closed.
  Owner-provided v3 snapshot is committed in S13 P3 9b2b10ce2.
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
