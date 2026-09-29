# Project Status

## Current Work

M5 T539 remains open. S1-S16 are accepted. S17 is admitted below.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S17; next unused S after accepted S16. |
| Admission And Approval | Coordinator admission under the owner's 2026-09-28 automatic-S authorization for chip extraction and Shared/NXVM reconnection. Targets: Shared video and NXVM memory/CPU observation. No MyNES executable dependency changes. |
| Objective | Close the CPU observation prerequisite: preview and timing reads preserve routed bytes without device-read or parity side effects, through one memory routing mechanism. Consume the CPU/memory boundary portion of the finite ledger; do not claim CPU extraction complete. |
| Non-goals | No instruction algorithm, timing grade, board clock, firmware, INI, CPU family or product identity change; no new framework, native pointer exposure or shared machine wrapper. |
| Reference Baseline | a3bb1e3f8; clean worktree, S16 accepted and eight 0539 artifacts. |
| Candidate Proposal | [T539 proposal](../proposals/m5-shared-chip-extraction.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md), [S17 contract](../etc/architecture/t539-s17-cpu-observation.md). |
| Files And ABI Surface | NXVM memory read-provider contract, CPU preview/timing callers, physical memory/ROM/video/board providers and their tests; Shared video bounded inspection operation and same-owner tests; applicable CMake, static gates, manifests and eight NXVM artifacts. |
| Applicable Rules | Task Reading Set; Execution, Architecture, Coding and Documentation rules; NXVM architecture/layout and source policy. Single routing/state owner, no private cross-chip access, Types vocabulary, retained original handler style and exact manifests. Existing owner-approved embedded-ROM artifact exception only. |
| Verification | Build both build/t539-s3/nxvm-x64 and nxvm-x86 with cmake --build --parallel 8; run ctest --test-dir each -L unit -j 4 --output-on-failure sequentially; full default integration -L integration -j 1 for each width; tools-off build/test in build/t539-s3/fdc-independent; specialized aggregate, six manifests, documentation governance and git diff --check. Rebuild/deploy all four NXVM profiles in both widths, inspect PE/hash/no compiler debug data, one boot per vendor/profile/width with existing bounded probe and no retirement observer. No repeated success trials. |
| Expected Markers | Ordinary reads retain side effects; inspect returns equal bytes without parity notification or EGA latch mutation; A20/reset/provider priorities and failure results preserved; preview has no page-table writes; timing-origin/formula tests unchanged; suites exit zero and artifacts identify 0539. |
| Asset Needs | Existing external BYOB build ROMs and integration media, unchanged masters and owner INIs. Unit fixtures are code-owned. Reuse the three bounded S16 build trees; no raw trace or new external asset. |
| Reporting Requirements | Confirm the boundary; report mechanism findings and failed verification honestly; evidence maps every provider and CPU observational read, before/after dispositions, actual diff/net lines, all receiving artifacts and known limits. |
| Stop Conditions | Stop for a required timing downgrade, changed instruction result, unrelated/shared consumer behavior or missing source/asset authority; revise the packet before material expansion. No acceptance on a local smoke alone. |
| Exit Criteria | Entire observation class reconciled, regression and complete receiving verification pass, no alternate address decoder or copy/restore of live device state, separate Shared/NXVM complete P commits pushed, coordinator actual-change review and governance closure. CPU extraction remains in T539. |
| Original Owner Request | Extract independent chips into x86/devices, preserve CPU handler style and all families, retain NXVM board ownership, automatically admit bounded S work and do not leave known in-scope defects behind. |
| Similar-Issue Sweep | Inspect CPU preview, timing descriptor rereads, all physical-memory read providers and diagnostic/frame read callers. Record each hit as fixed or genuinely operational; no silent diagnostic-to-bus fallback. Regress and statically guard preview/timing use of the observation path. |

## Retained Progress

| Task | Progress |
| --- | --- |
| T539 S16 | Accepted: Shared P1 5fa831a2b and NXVM P2 9783297fb extract the sole opaque FPU, retain CPU pairing/operand ownership and remove the old implementation. Both widths pass 368/368 units and 20/20 default integrations; tools-off 44/44, six manifests, specialized/document gates and eight artifact identities pass. |

Coordinator actual-commit review accepts the complete three-file FPU ledger
row. Arithmetic/timing bodies retain original algorithms; CPU pairing, opaque
lifetime, original-case migration, public deadline observations, receiving
builds and target-separated commits were reviewed directly. The paused-frame
integration correction preserves every cell assertion. Model40's initial
instrumented timeouts are retained: one controlled contrast per width with
only the existing retirement observer disabled reached installer-ready under
the unchanged 90-second bound. No guest workaround, timing downgrade or new
full-x87 claim was introduced.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S16 boundary](../etc/architecture/t539-s16-fpu-extraction.md),
[S16 evidence](../etc/evidence/t539-s16-fpu-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, limits and delivery.
CPU and all remaining inventory dispositions still belong to T539; none is
silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
9783297fb with unchanged owner INIs. S16 evidence records hashes, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba, video at
522d0b27f and FPU at 5fa831a2b. MyNES retains its unchanged 0043 pair: its link
inputs do not include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S16.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
