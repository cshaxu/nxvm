# Project Status

## Current Work

M5 T539 remains open. S1-S21 are accepted; S22 is admitted below.
S23-S40 remain planned in the
[CPU work packages](../etc/architecture/t539-cpu-work-packages.md).

S21 implementation P1 `1049b9021` separates LEA/MOVX instruction invariants
from board composition. Coordinator actual-commit review accepts all 53
original cases and eight additional intended MOVX opcode cases. Complete units
pass 373/373 on each width; 66 specialized steps, six unchanged manifests and
documentation/diff checks pass. The one concurrent native-window test failure
and passing isolated x86 rerun are recorded, not concealed or claimed fixed.
See [S21 evidence](../etc/evidence/t539-s21-lea-movx-migration.md) and
[TODO](TODO.md).

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 98 original direct private-test consumers and include dependents to
S22-S37. Embedded CPU lifetime remains until S38; physical Shared relocation
is S39; whole CPU acceptance is S40. S32 owns the unresolved 32-bit BOUND
observation. None is silently closed or transferred to the next T.

## Active S22 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S22, baseline S21 P2 bf8dd127c. Target NXVM only. |
| Admission And Approval | Owner's automatic-S authorization and bounded CPU decomposition request; coordinator admits the GPR MOV/MOFFS receiving batch. No new Shared API or product behavior. |
| Objective | Separate all GPR MOV/MOFFS instruction invariants from real board composition, removing private board-CPU access in the two original tests. |
| Non-goals | No CPU algorithm/timing changes, public mutable CPU access, opaque lifetime cutover, Shared relocation, new framework, MyNES/INI/asset edits. |
| Reference Baseline | bf8dd127c, clean intake; core_machine_gpr_mov_smoke.c and core_machine_moffs_smoke.c total 1,407 lines. Original family/case map is in S22 evidence. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md), [work packages](../etc/architecture/t539-cpu-work-packages.md), [inventory](../etc/evidence/t539-cpu-incremental-inventory.md). |
| Files And ABI Surface | Two original board tests, receiving CPU-owned tests and existing CPU fixture; necessary NXVM CMake/gate classification and task records. No production ABI changes. |
| Applicable Rules | Product guide reading set; shared Execution/Architecture/Coding/Document; NXVM Architecture/Coding/Roadmap. CPU owns hidden state; board uses public operations and copied observations. No imports or firmware changes. |
| Verification | Build both existing NXVM trees and run each complete unit suite without cross-tree desktop overlap. Run specialized gates, six manifests, documentation and diff checks. Develop against the four changed test targets. |
| Expected Markers | Retain original GPR MOV T316/S31 and T401/S13/S47/S58 and MOFFS T316/S30/T401/S14; preserve 277 original loop contexts, register/memory rollback and real IRQ frames. CPU-only tests link x86-cpu, not core-machine. |
| Asset Needs | Repository-owned instruction bytes only. No new build tree. Current eight 0539 EXEs stay valid if executable inputs remain unchanged; rebuild affected pairs if that changes. |
| Reporting Requirements | Report ownership decision, all original-to-receiver mappings, discovered differences, counted source/test changes and actual verification; complete implementation P then coordinator actual-commit review. |
| Stop Conditions | Stop for required new public contracts, lost profile/rollback case, instruction result or timing-grade changes, unrelated edits, or production repair beyond this migration. |
| Exit Criteria | All 277 original contexts retained; chip cases independently executable; four protected-limit and four IRQ board cases use no private CPU layout. Complete verification and actual-diff review pass; target-scoped delivery pushed. CPU extraction itself remains open. |
| Original Owner Request | Independent chip extraction, original handler/table style and behavior preserved; bounded S tasks, automatic admission. |
| Similar-Issue Sweep | Search both files and includers for executor_cpu, executor_memory, legacy fixture and private includes; preserve byte/width/profile matrices and all terminal-fault/partial-effect assertions. Remaining raw consumers retain S23-S37 receivers. |

## Accepted Progress

| Task | Progress |
| --- | --- |
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
and verification limits. S19-S21 changed no executable inputs and require no
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
