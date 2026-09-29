# Project Status

## Current Work

M5 T539 remains open. S1-S22 are accepted; S23 (XCHG) is admitted below.
S24-S40 remain planned in the
[CPU work packages](../etc/architecture/t539-cpu-work-packages.md).

S22 implementation P1 `9e5382872` separates GPR MOV/MOFFS instruction
invariants from board composition. Coordinator actual-commit review accepts
all 277 original contexts: 269 chip and eight board cases. Complete units pass
375/375 on each width; 66 specialized steps, six unchanged manifests and
documentation/diff checks pass. Ten test/build paths have a net reduction of
134 lines; production and executable inputs are unchanged.
See [S22 evidence](../etc/evidence/t539-s22-mov-moffs-migration.md).

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 96 original direct private-test consumers and include dependents to
S23-S37. Embedded CPU lifetime remains until S38; physical Shared relocation
is S39; whole CPU acceptance is S40. S32 owns the unresolved 32-bit BOUND
observation. None is silently closed or transferred to the next T.

## Active S23 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S23; baseline S22 P2 8dd52d43f. Target NXVM only. |
| Admission And Approval | Owner automatic-S authorization and bounded CPU decomposition request; coordinator admits the XCHG migration without new Shared or product behavior. |
| Objective | Separate XCHG chip invariants from board composition and eliminate private CPU access in the original board test. |
| Non-goals | No CPU algorithm/timing changes, Shared relocation, new public test API, lifetime cutover, MyNES/INI/asset changes or new device framework. |
| Reference Baseline | 8dd52d43f; clean intake. Original core_machine_xchg_smoke.c has 876 lines, twelve test families and 101 instruction contexts. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md), [work packages](../etc/architecture/t539-cpu-work-packages.md), [inventory](../etc/evidence/t539-cpu-incremental-inventory.md), [S23 evidence](../etc/evidence/t539-s23-xchg-migration.md). |
| Files And ABI Surface | Original board test, CPU-owned receiving test, existing CPU fixture if needed, NXVM build/gate classifications and task records. No production ABI change. |
| Applicable Rules | Product reading set; shared Execution/Architecture/Coding/Document and NXVM Architecture/Coding. One chip state owner, public copied board observations, original table/handler style and coverage preserved. No imports. |
| Verification | Build existing x64/x86 trees; complete unit suites sequentially to avoid overlapping native desktop tests. Specialized gates, six manifests, documentation/diff checks. Transient focused selection is the two XCHG targets and CPU-boundary negative test. |
| Expected Markers | Preserve T316/S27/S28, T401/S12/S46 and all original arrays/loops/assertions. CPU target links x86-cpu only. Retain three protected fault cases and two real PIC frame cases. |
| Asset Needs | Repository-owned instruction bytes only. Retain three build/t539-s3 trees and recovery patch. No EXE rebuild when link inputs remain unchanged; rebuild affected pairs if that changes. |
| Reporting Requirements | Report case split, fixture-mechanism changes, actual verification, size counts and complete-P push; coordinator separately reviews actual commit. |
| Stop Conditions | Stop for lost assertions, behavior/timing-grade differences, required new public contracts, unrelated edits or production repairs beyond this batch. |
| Exit Criteria | All 101 contexts retained; 96 chip instruction contexts independent of board; five board cases use no private CPU. Full verification and actual-diff review pass with target-scoped pushed delivery. CPU row remains open. |
| Original Owner Request | Extract independent chips preserving CPU handlers, semantics and tests; bounded traceable S deliveries with automatic admission. |
| Similar-Issue Sweep | Review all twelve XCHG families and source includers, private CPU/cache/memory access and sequential instruction setup. Preserve writable/limit fault rollback and real IRQ frames; other consumers retain S24-S37 receivers. |

## Accepted Progress

| Task | Progress |
| --- | --- |
| T539 S22 | Accepted: NXVM P1 9e5382872 migrates GPR MOV/MOFFS test ownership. All 277 original contexts retained; units 375/375 per width. No production or asset change. |
| T539 S21 | Accepted: NXVM P1 1049b9021 migrates LEA/MOVX test ownership and divides the oversized instruction batch. Units 373/373 per width. Production, EXEs and INIs unchanged. |
| T539 S20 | Accepted: NXVM P1 af06a6259 qualifies copied observations, debug/reset adapters and board access. Units 371/371 per width. |
| T539 S19 | Accepted: NXVM P1 f1b43af46 qualifies CPU bus transactions, failure effects and imports. Units 371/371 per width. |
| T539 S18 | Accepted: NXVM P1 0067d80c4 restores the incremental baseline and preserves pending migrations. Units 370/370 per width; default integration 20/20 per width; tools-off 45/45; six vendor boots once. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain the
complete scope, original requirements, earlier acceptance and receiving proof.
The historical S18-S20 prospective numbering is superseded only for unadmitted
packages by the current work plan.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
0067d80c4 with unchanged owner INIs. S18 evidence records hashes, PE architecture
and verification limits. S19-S22 changed no executable inputs and require no
new artifact. Both reusable NXVM trees remain configured for default; the three
bounded build/t539-s3 trees and S18 recovery patch remain needed for later CPU
batches. Run native desktop test suites without cross-tree overlap.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba, video at
cadaf0990 and FPU at 5fa831a2b. MyNES retains its unchanged 0043 pair: its link
inputs do not include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by the CPU batches.

The seven [Queue](QUEUE.md) candidates retain dependency order. Acceptance does
not claim indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
