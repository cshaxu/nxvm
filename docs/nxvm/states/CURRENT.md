# Project Status

## Current Work

M5 T539 remains open. S1-S17 are accepted. The owner's 2026-09-29 amendment
replaces the oversized S18 delivery with [S18-S32 packages](../etc/architecture/t539-cpu-work-packages.md).
S18 remains active for green-baseline recovery; S19-S32 are planned, not accepted.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S18, amended by owner; S19-S32 reserved in the CPU work-package plan. Only S18 is active. |
| Admission And Approval | Owner's 2026-09-28 automatic-S authorization for T539 and 2026-09-29 request to split S18; coordinator admits baseline recovery and S18-S32 staging at 25ec0f6c3. Existing embedded-ROM artifact exception remains. |
| Objective | Recover a green incremental CPU migration baseline and map every pending edit/original test case to S18-S32; defer the allocation/layout cutover if needed without adding a second state owner or private-pointer bridge. |
| Non-goals | No new CPU family, opcode algorithm rewrite, timing-grade change, common PC-board migration, App split, MyNES change, INI change or new external assets. |
| Reference Baseline | Clean master/origin master 25ec0f6c3; S17 observation route and committed eight 0539 artifacts. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md) and [S18-S32 staging](../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | NXVM pending CPU/board callers, timing/diagnostics, firmware hook removal, tests/build/tools/docs and eight artifacts. Retain the existing embedded owner until S30; no new private-pointer bridge. Shared relocation/manifests belong to S31, not this recovery. No Lib/Common/MyNES edits. |
| Applicable Rules | EXECUTION lifecycle and target-separated P commits; DOCUMENT authority/links; architecture single owner, neutral dependencies and opaque boundaries; coding Types, original handler/table style and no parallel path; source policy preserves notices and existing ROM exception. Architecture/coding skills apply. |
| Verification | Serial build trees: complete NXVM x64/x86 unit and default integration suites; standalone tools-off x86 suite; CPU timing/exception/FPU/bus/observation scenarios; specialized gates; six manifests, corpus and documentation gates, diff check; XT/AT/Model40 boot once per width with existing observer-free probe; eight optimized stripped 0539 artifacts and unchanged INIs. |
| Expected Markers | Complete unit corpus builds and passes with original scenarios; every pending consumer has a receiving S; one CPU owner and real INT/IVT path; bus trace order and partial effects unchanged; no timing downgrade; vendor installer/DOS completion within existing bounds, not timeout-as-success. Independent CPU packaging is not claimed before S31. |
| Asset Needs | Existing external BYOB and media inputs only. Keep the two NXVM and standalone build/t539-s3 trees for this batch; no raw trace unless separately bounded in this packet. |
| Reporting Requirements | Report retained/deferred edits, their recoverable inventory, original-case mapping and actual verification. No broken-baseline P, lost work or false CPU acceptance. Earlier narrow results below are historical work-in-progress, not full-suite proof. |
| Stop Conditions | Stop affected work for an unapproved behavior/timing loss, asset/license change, irreconcilable external caller or architecture contract outside the reviewed boundary; do not hide it by weakening tests. |
| Exit Criteria | Full unit build/run restored; pending work assigned without loss; no duplicate CPU or new compatibility bridge; required affected builds/artifacts and target-separated P review/push complete. The nine-file extraction remains open through S32. |
| Original Owner Request | Extract independent chips to src/x86/devices, retain NXVM board assembly, preserve all CPU implementations and original instruction style; automatically admit S batches and commit the embedded-ROM EXEs. |
| Similar-Issue Sweep | Inspect all tracked CPU/private-state, memory/port/PIC/transaction, firmware interception, timing catalog and diagnostic consumers across src/test/cmake/tools. Classify chip versus board ownership, migrate every live caller and add mechanically enforceable boundary checks. MyNES has no x86 device link input and is excluded with that proof. |

S18 has recovered the complete x64 build by retaining the original single
embedded CPU/decoder until S30. The incomplete interrupt-entry fixture cutover
is deferred with its includers to S27, not replaced by a second test API. No
CPU instruction or timing expectation was relaxed. The complete x64 unit run
passes 370/370 (224.45 seconds); x86 full build and units also pass 370/370
(37.82 seconds). Default integrations pass 20/20 per width and six manifests
pass. The specialized aggregate now passes after seven ownership/classification
failures were repaired; two duplicated test lifecycle tails were consolidated.
After that test change, fresh full unit runs pass 370/370 on each width;
the tools-off standalone suite passes 45/45. All six vendor boots pass once
with installer-ready; all eight 0539 artifacts are rebuilt and identity-checked,
with unchanged INIs. Both build trees are restored to default. Executor actual-diff
review is complete, including the added CPU-owned full-cache regression; latest
unit logs confirm 370/370 per width. Implementation delivery is ready for commit
and coordinator review; this is not acceptance.

The [incremental inventory](../etc/evidence/t539-cpu-incremental-inventory.md)
records the recoverable pre-recovery diff, retained/deferred edits and exact
remaining test consumers. The [S18 evidence](../etc/evidence/t539-s18-cpu-extraction.md)
retains earlier narrow results and chronology; those precede this recovery
and do not constitute current full-suite or CPU-extraction acceptance.
No S18 P has been committed or pushed.

## Retained Progress

| Task | Progress |
| --- | --- |
| T539 S17 | Accepted: Shared P1 cadaf0990 and NXVM P2 f85888d3e make CPU preview/timing and display backing inspection side-effect-free through the existing memory resolver. Both widths pass 370/370 units and 20/20 default integrations; tools-off 45/45, six vendor boots once, six manifests, specialized/document gates and eight artifact identities pass. |

Coordinator actual-commit review accepts the observation prerequisite, not the
CPU extraction row. The sole memory route, explicit provider intent, local
video latch calculation, unchanged operational reads and CPU handler/timing
bodies, original tests and complete receiving proof were reviewed directly.
The missing new-fixture classification was corrected without weakening the
gate. Model40 passes both widths within 90 seconds using the existing
observer-free probe. No guest workaround or timing downgrade was introduced.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S17 boundary](../etc/architecture/t539-s17-cpu-observation.md),
[S17 evidence](../etc/evidence/t539-s17-cpu-observation.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, limits and delivery.
CPU and all remaining inventory dispositions still belong to T539; none is
silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
f85888d3e with unchanged owner INIs. S17 evidence records hashes, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba, video at
cadaf0990 and FPU at 5fa831a2b. MyNES retains its unchanged 0043 pair: its link
inputs do not include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S17.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
