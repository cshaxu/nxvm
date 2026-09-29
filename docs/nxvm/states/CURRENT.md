# Project Status

## Current Work

M5 T539 remains open. S1-S23 are accepted; S24 (GPR stack) is active,
implemented and verified, awaiting coordinator actual-commit review.
S25-S40 remain planned in the
[CPU work packages](../etc/architecture/t539-cpu-work-packages.md).

## Active S24 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S24, following accepted S23 P2 a9ca9a6f2; no identifier reused. |
| Admission And Approval | Owner's 2026-09-28 automatic-S authorization and 2026-09-29 CPU decomposition request; coordinator admits the planned GPR-stack batch. NXVM target only; no new exception. |
| Objective | Separate CPU-owned PUSH/POP, immediate PUSH and PUSHA/POPA tests from PC board fault/IRQ composition, preserving every original context and assertion. |
| Non-goals | No production instruction/timing change, Shared relocation, opaque allocation, new debug API, firmware/media/INI/EXE change, or MyNES change. |
| Reference Baseline | a9ca9a6f2; clean worktree; S23 units 376/376 per width. CPU work plan, CPU boundary and incremental inventory linked below retain the final extraction requirements. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md), [S24 work package](../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Three original core_machine_gpr_push_pop/push_immediate/pusha_popa_smoke.c files, three CPU-owned receivers, NXVM CMake/authority/fixture/negative checks, and task evidence. Existing public CPU operations/copies only; no ABI change. |
| Applicable Rules | Shared execution, architecture, coding and documentation rules; NXVM design and layout; architecture-governance and coding-governance skills. Single state owner, no private CPU access from migrated board tests, original handler style/coverage, one NXVM target per P. |
| Verification | Fresh x64/x86 retained-tree builds; full unit suites sequential between widths; verify-current-specialized-gates; all six manifest checks; documentation gate and git diff --check. Transient selection: the three original board and three new CPU stack tests. |
| Expected Markers | Existing instruction/board assertions pass; complete unit count increases only by the three CPU receivers; private CPU dependencies disappear from all three board tests and remain only at the CPU owner; no source/assets diff. |
| Asset Needs | Repository-owned test values only. Keep the three build/t539-s3 trees and S18 recovery patch for following CPU packages; external integration and new EXEs are not required for test-only inputs. |
| Reporting Requirements | Confirm boundary, report original-case reconciliation and substantive failures, then provide pushed P, complete verification, code-size delta and remaining CPU scope. [S24 evidence](../etc/evidence/t539-s24-gpr-stack-migration.md) owns the case map. |
| Stop Conditions | Stop for uncovered case loss, new public interface or behavior/timing decision, unrelated worktree overlap, or need to change Shared/MyNES/production inputs; do not weaken assertions to pass. |
| Exit Criteria | All S24 original contexts have explicit CPU/board receivers, three old private dependencies removed, required checks pass, actual-diff review accepts the complete delivery, implementation P and coordinator governance P pushed. T539 remains open. |
| Original Owner Request | Extract independent chips while retaining all CPU families and original style/semantics; split oversized S18 into bounded traceable S deliveries; automatically admit subsequent S tasks. |
| Similar-Issue Sweep | Search all three originals, their include dependents, NXVM test/build registries and boundary gates for executor_cpu, legacy CPU fixture and private cpu.h imports. Preserve artificial cache assertions at CPU owner or equivalent complete copied observations; retain actual PIC/physical-memory composition. Other inventoried consumers remain assigned to S25-S37. |

## S24 Executor Result

All 198 original stack instruction contexts have receivers: 190 CPU executions
and 16 board executions, with eight protected faults retaining complementary
private-cache and real-board assertions. Complete units pass 379/379 on each
width; specialized gates, all six unchanged manifests and documentation/diff
checks pass. Eleven test/build paths add 1,692/remove 1,232 lines. No production
or EXE input changes. See [S24 evidence](../etc/evidence/t539-s24-gpr-stack-migration.md).

## Retained S23 Baseline

S23 implementation P1 `5c2835936` separates XCHG chip invariants from board
composition. Coordinator actual-commit review accepts all 101 original
instruction contexts: 96 chip and five board cases. Complete units pass
376/376 on each width; 66 specialized steps, six unchanged manifests and
documentation/diff checks pass. Seven test/build paths have a net reduction
of 79 lines. Production and executable inputs remain unchanged.
See [S23 evidence](../etc/evidence/t539-s23-xchg-migration.md).

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 92 original direct private-test consumers and include dependents to
S25-S37 after the pending S24 delivery. Embedded CPU lifetime remains until S38; physical Shared relocation
is S39; whole CPU acceptance is S40. S32 owns the unresolved 32-bit BOUND
observation. None is silently closed or transferred to the next T.

## Accepted Progress

| Task | Progress |
| --- | --- |
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
and verification limits. S19-S23 changed no executable inputs and require no
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
