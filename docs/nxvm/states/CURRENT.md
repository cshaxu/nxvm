# Project Status

## Current Work

M5 T539 remains open. S1-S25 are accepted; S26 (segment stack) is active.
The [CPU work packages](../etc/architecture/t539-cpu-work-packages.md)
retain S27-S40 as pending, not accepted CPU extraction.

## S26 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S26; next unused S after accepted S25 P2 430abfc54. Target NXVM only; no Shared or MyNES edits. |
| Admission And Approval | Owner automatic-S authorization of 2026-09-28 and CPU decomposition amendment of 2026-09-29; coordinator admits this bounded segment-stack package on 2026-09-29. One-session coordinator/executor roles; no new exception. |
| Objective | Consume the two S26 inventory rows: separate all 164 original segment-stack contexts into CPU-owned instruction tests and public board receivers without losing private-cache, fault, or IRQ-shadow assertions. |
| Non-goals | No CPU algorithm, production API, timing grade, CPU lifetime, physical Shared relocation, firmware, INI, artifact or MyNES change. No new public accessor for private cache state. |
| Reference Baseline | Clean 430abfc54; S25 accepted units 380/380 per width. Original legacy_sreg_stack has 144 contexts; fs_gs_stack has 20. Read the CPU boundary and incremental inventory linked below. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [CPU work packages](../etc/architecture/t539-cpu-work-packages.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md). |
| Files And ABI Surface | The two core_machine segment-stack smoke sources, new cpu segment-stack receivers, relevant NXVM CMake registrations and boundary/lifecycle/negative gates; NXVM packet, inventory, history and evidence. No production or public ABI changes. |
| Applicable Rules | Product guide Task Reading Set; shared Execution, Architecture, Coding and Document rules; NXVM Architecture/Coding. Preserve original handler/test tables, one CPU owner, full case coverage and private-test ownership. Skills: architecture-governance then coding-governance. No external source/asset operation. |
| Verification | Full x64 and x86 builds and complete unit suites, sequential native desktop suites; all specialized gates, six unchanged manifests, documentation governance and diff check. Transient focused selection: old/new legacy and FS/GS segment-stack targets plus CPU bus boundary negatives. |
| Expected Markers | Both original suites retain their coverage; new CPU targets pass linked only to x86-cpu; both board files reject private CPU imports/access. Map original cases and every complementary board receiver explicitly. |
| Asset Needs | Repository-only code inputs. Retain existing build/t539-s3 trees and S18 recovery patch for subsequent batches. No external ROM/media or executable-input change; deployed 0539 artifacts remain current. |
| Reporting Requirements | Confirm boundary before implementation; report case-map and verification discoveries; delivery reports exact counted paths/additions/deletions, complete-P hash/push and proof. CPU extraction remains pending until S40. |
| Stop Conditions | Stop/revise for lost coverage, newly required production API, behavior/timing change, include-dependent caller outside the package, or evidence that the batch exceeds its bounded scope. Failed checks prevent commit/acceptance, not diagnosis. |
| Exit Criteria | All 164 original contexts accounted for, complete cache assertions retained at CPU owner, real PIC/fault board obligations preserved through public observations, old private dependencies removed from both board tests, all verification and actual-diff review pass, complete implementation P pushed then coordinator closure P pushed. |
| Original Owner Request | Split oversized S18 into individually traceable S tasks while completing independent chips, preserving original CPU semantics/style and avoiding duplicate paths; automatically admit each S. |
| Similar-Issue Sweep | Search both original files, all include callers, CMake inventories and negative gates for executor_cpu, legacy fixture and private CPU imports. Invalid selector/cache constructions remain CPU-local; real PIC wiring stays board-owned. The other 89 original consumers remain assigned S27-S37, not silently accepted. |

## Retained S25 Baseline

S26 executor delivery: all 164 original contexts have CPU-only receivers,
with 18 complementary board executions. Full units pass 382/382 per width;
66 specialized gates and six unchanged manifests pass. Nine test/build paths
add 1,107/remove 783 lines. No production or executable inputs changed.
See [S26 evidence](../etc/evidence/t539-s26-segment-stack-migration.md).
Coordinator actual-commit acceptance remains pending.

Coordinator actual-commit review accepts S25 implementation P1 `2cb8b64f7`.
ENTER/LEAVE retains all 53 original contexts: 51 CPU and four board executions,
including two complementary protected-fault receivers. Original instruction
tables and complete cache/register comparison helpers remain. Complete units
pass 380/380 per width; all 66 specialized gates, six unchanged manifests and
documentation/diff checks pass. Seven test/build paths add 679/remove 518
lines, net +161. No production or executable input changes.
See [S25 evidence](../etc/evidence/t539-s25-enter-leave-migration.md).

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 89 original direct private-test consumers and include dependents to
S27-S37 after the pending S26 worktree migration. Embedded CPU lifetime remains until S38; physical Shared relocation
is S39; whole CPU acceptance is S40. S32 owns the unresolved 32-bit BOUND
observation. None is silently closed or transferred to the next T.

## Accepted Progress

| Task | Progress |
| --- | --- |
| T539 S25 | Accepted: NXVM P1 2cb8b64f7 migrates ENTER/LEAVE test ownership. All 53 original contexts retained; units 380/380 per width. No production or asset change. |
| T539 S24 | Accepted: NXVM P1 ff09b22a3 migrates GPR stack test ownership. All 198 original contexts retained; units 379/379 per width. No production or asset change. |
| T539 S23 | Accepted: NXVM P1 5c2835936 migrates XCHG test ownership. All 101 original instruction contexts retained; units 376/376 per width. No production or asset change. |
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
and verification limits. S19-S25 changed no executable inputs and require no
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
