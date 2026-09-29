# Project Status

## Current Work

M5 T539 remains open. S1-S30 are accepted; S31 is active. The former
eleven-file, 7,000-plus-line arithmetic assignment is split into S30-S35
under the existing automatic-S authorization; S32-S45 remain pending.
The [CPU work packages](../etc/architecture/t539-cpu-work-packages.md)
retain the remaining CPU work as pending, not accepted CPU extraction.

## S30 Acceptance

Actual pushed NXVM P1 `442088410` has exactly 23 scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 549 original
contexts retain one receiver: 539 CPU-owned and ten board-owned. The 19
code/test/build paths add 1,429/remove 1,506 lines (net -77); six CPU tests
link only `x86-cpu`, while six board tests retain real faults or IRQ through
Core-machine. Complete x64/x86 units pass 395/395 each, 66 specialized gates
pass per width, six unchanged Shared manifests verify, and documentation
governance passes. No production/API or executable input changed. See
[S30 evidence](../etc/evidence/t539-s30-bit-condition-extension-migration.md).
The S30 packet is closed; 76 original private-test consumers remain assigned
to S31-S42. S43-S45 still own lifetime, physical relocation and whole-CPU
acceptance.

## S31 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S31 after S30 P2 `e8511d6d0`; NXVM target only. |
| Admission And Approval | Coordinator admission under the owner's automatic-S authorization; one session switches executor/coordinator roles. |
| Objective | Move CPU-owned immediate IMUL and Group-2 rotate/shift assertions from two mixed sources to CPU-only receivers; retain actual protected faults and PIC IRQ at the board. |
| Non-goals | No CPU production/timing change, new public ABI, S32-S35 ALU work, Shared relocation, opaque lifetime, firmware, INI, MyNES or executable-input change. |
| Reference Baseline | Clean pushed `e8511d6d0`, complete units 395/395 per width. [CPU work packages](../etc/architecture/t539-cpu-work-packages.md) assign only `core_machine_imul_immediate_s56_smoke.c` and `core_machine_rotate_smoke.c` to S31. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md), [inventory](../etc/evidence/t539-cpu-incremental-inventory.md). |
| Files And ABI Surface | The two original tests, CPU-only receivers, test-local fixtures only when deleting repeated setup, NXVM test registration/boundary gates and task evidence. Preserve every original profile, program, case and assertion receiver. |
| Applicable Rules | Product reading set, shared/NXVM architecture and coding, execution and documentation rules. Synthetic CPU cache/flags/results stay CPU-owned; real descriptor delivery, physical memory and PIC IRQ stay board-owned. No external-source operation. |
| Verification | Full x64/x86 builds and repository-only unit suites, specialized gates, six unchanged manifests, documentation/diff checks. Focused selection is local to S31 and not a durable gate. |
| Expected Markers | CPU receivers link only `x86-cpu`; board receivers use public machine operations without private CPU state; one execution path per original case. |
| Asset Needs | Repository-only inputs and existing build trees; preserve the S18 local recovery artifact. No ROM/media/INI/EXE input change. |
| Reporting Requirements | Original-case receiving map, exact changed paths and code line counts, dual-width results, pushed implementation and actual-commit review. |
| Stop Conditions | Lost original case, unaccounted includer, new production/API requirement, or scope beyond the two sources requires packet revision before continuing. |
| Exit Criteria | CPU/board ownership proved, all original scenarios retained, full unit/gates and actual-commit acceptance, pushed implementation/governance Ps. |
| Original Owner Request | Split oversized CPU migration into traceable S tasks with automatic admission, preserving original semantics, code style and single-owner architecture. |
| Similar-Issue Sweep | Inspect both suites for private fixture access, profile/timing claims, synthetic segment-cache setup, real board faults and IRQ, and Group-2 #UD polarity. Leave S32-S35 assigned, not presumed done. |

## S29 Acceptance

Actual pushed NXVM P1 `86fe95201` has exactly 13 scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 28 original
operand contexts and all twelve original prefix groups retain CPU or board
receivers; two fault contexts have complementary observations. x64 and x86
builds and complete units pass 389/389 each, along with 66 specialized gates,
extended CPU-boundary negatives, six unchanged manifests and documentation
governance. The nine test/build paths add 1,514/remove 1,311 lines; no
production/API or executable input changed. See
[S29 evidence](../etc/evidence/t539-s29-operand-prefix-migration.md). S29 is
accepted; 82 original private consumers remain assigned to S30-S42. S43 owns
opaque lifetime, S44 physical Shared relocation and S45 whole-CPU acceptance.

## S28 Acceptance

Actual-commit review accepts pushed NXVM P1 `fa092b95e`: the 244 original
execution contexts and three metadata queries retain CPU/board receivers;
the inverted 286 test result and incorrect EAX expectation are corrected.
Full x64/x86 units pass 387/387 each, with 66 specialized gates, six unchanged
manifests and documentation/diff checks. Ten test/build paths add 2,065/remove
1,530 lines, net +535; no production or executable input changed. See
[S28 evidence](../etc/evidence/t539-s28-segment-migration.md). The S28 packet
is closed, not carried into S29.

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 84 original direct private-test consumers and include dependents to
S30-S42. Embedded CPU lifetime remains until S43; physical Shared relocation
is S44; whole CPU acceptance is S45. S37 owns the unresolved 32-bit BOUND
observation. None is silently closed or transferred to the next T.

## Accepted Progress

| Task | Progress |
| --- | --- |
| T539 S30 | Accepted: NXVM P1 442088410 migrates bit/condition/extension ownership. All 549 original contexts retain CPU/board receivers; units 395/395 per width. No production or asset change. |
| T539 S29 | Accepted: NXVM P1 86fe95201 migrates operand/address and S64 prefix ownership. All 28 operand contexts and twelve prefix groups retain receivers; units 389/389 per width. No production or asset change. |
| T539 S28 | Accepted: NXVM P1 fa092b95e migrates segment selector/SREG MOV test ownership. All 244 original contexts and three queries retained; units 387/387 per width. No production or asset change. |
| T539 S27 | Accepted: NXVM P1 42d6c86e0 migrates far-pointer test ownership. All 117 original contexts retained; units 385/385 per width. No production or asset change. |
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
and verification limits. S19-S30 changed no executable inputs and require no
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
