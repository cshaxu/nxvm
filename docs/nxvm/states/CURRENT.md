# Project Status

## Current Work

M5 T539 remains open. S1-S26 are accepted; S27 is active.
The [CPU work packages](../etc/architecture/t539-cpu-work-packages.md)
retain S27-S40 as pending, not accepted CPU extraction.

## S27 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S27 after accepted S26 P2 9a4de7adf; NXVM target only. |
| Admission And Approval | Owner automatic-S authorization of 2026-09-28 and CPU decomposition amendment of 2026-09-29; coordinator admits this bounded package on 2026-09-29, single-session dual roles. |
| Objective | Migrate the three far-pointer-load inventory rows without losing their 117 original contexts; remove board-private CPU access while retaining instruction/cache assertions and PIC/fault receivers. |
| Non-goals | No CPU semantics, timing grade, public API, Shared relocation, lifetime cutover, firmware, INI, MyNES or artifact change. |
| Reference Baseline | Clean S26 P2 9a4de7adf; units 382/382 per width. Original LES/LDS S41 has 68 contexts, LES/LDS 22 and LSS/LFS/LGS 27. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [CPU work packages](../etc/architecture/t539-cpu-work-packages.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md), [inventory](../etc/evidence/t539-cpu-incremental-inventory.md). |
| Files And ABI Surface | Three original far-pointer-load smoke files, CPU-owned receivers, relevant NXVM CMake registrations/lifecycle/boundary gates, packet/history/evidence. No public ABI change. |
| Applicable Rules | Product guide Task Reading Set, NXVM Architecture/Coding, shared Execution/Architecture/Coding/Document rules; architecture-governance then coding-governance skills. Preserve original tables, single owner and all contexts. No external-source operation. |
| Verification | Full x64/x86 builds and complete unit suites, native desktop suites sequential; specialized gates, six unchanged manifests, documentation governance and diff check. Transient focused selection: three old/new far-pointer targets and CPU bus negative controls. |
| Expected Markers | CPU receivers link only x86-cpu; original board files reject private CPU imports/access; original contexts map explicitly to CPU/board receivers, including overlapping matrices. |
| Asset Needs | Repository-only test inputs. Retain existing build/t539-s3 trees and S18 recovery patch; EXE inputs unchanged, no external assets or INI changes. |
| Reporting Requirements | Confirm boundary, report discoveries, original-case mapping, counted paths and line delta, exact verification and pushed complete implementation P before coordinator review. |
| Stop Conditions | Lost cases, new production interface/behavior requirement, unaccounted includer, or oversized scope requires packet revision. Failures prevent acceptance and require diagnosis. |
| Exit Criteria | All 117 contexts retained; private cache predicates CPU-owned, PIC/shadow and machine faults board-owned; full verification and actual-commit review pass; implementation and governance Ps pushed. |
| Original Owner Request | Decompose oversized S18 into independently traceable S tasks while completing independent CPU extraction without style/semantic loss; automatically admit each S. |
| Similar-Issue Sweep | Search all three originals, their includers and CMake inventories for private CPU imports/access. Preserve inconsistent selector/cache preconditions at CPU owner; inspect skipped-case fixture lifetime. Remaining consumers retain S28-S37 receivers. |

## Retained S26 Baseline

S27 executor delivery retains all 117 original contexts, with 110 CPU and 20
board executions (13 complementary fault contexts). Complete units pass 385/385
per width; 66 specialized gates and six unchanged manifests pass. Eleven
test/build paths add 1,294/remove 1,017 lines. No executable inputs changed.
The remaining original private consumers number 86, assigned S28-S37.
See [S27 evidence](../etc/evidence/t539-s27-far-pointer-migration.md).
Coordinator actual-commit acceptance is pending.

Coordinator actual-commit review accepts S26 implementation P1 `587a91af9`.
All 164 original segment-stack contexts remain in CPU-only tests, with 18
complementary board executions. Private-cache predicates stay CPU-owned; real
PIC acknowledgement, SS interrupt shadow and machine faults retain board proof.
Complete units pass 382/382 per width; all 66 specialized gates, six unchanged
manifests and documentation/diff checks pass. Nine test/build paths add
1,107/remove 783 lines, net +324. No production or executable inputs changed.
See [S26 evidence](../etc/evidence/t539-s26-segment-stack-migration.md).
S27 (LES/LDS and LSS/LFS/LGS) is the next planned package.

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 89 original direct private-test consumers and include dependents to
S27-S37. Embedded CPU lifetime remains until S38; physical Shared relocation
is S39; whole CPU acceptance is S40. S32 owns the unresolved 32-bit BOUND
observation. None is silently closed or transferred to the next T.

## Accepted Progress

| Task | Progress |
| --- | --- |
| T539 S26 | Accepted: NXVM P1 587a91af9 migrates segment-stack test ownership. All 164 original contexts retained; units 382/382 per width. No production or asset change. |
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
and verification limits. S19-S26 changed no executable inputs and require no
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
