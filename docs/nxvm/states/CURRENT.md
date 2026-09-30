# Project Status

## Current Work

M5 T539 remains open. S1-S41 are accepted; S42 is active under automatic
admission. S42 P1 `f598c6d55` and P2 `dd7d20cbf` establish its CPU and
public-board receivers, but the three original sources remain until the real
CPL-transition receiver replaces their remaining privilege coverage. The former eleven-file, 7,000-plus-line arithmetic assignment is
split into S30-S35 under the existing automatic-S authorization. At S36 intake,
its oversized FLAGS/string/port row was divided into S36-S39. At S40 intake,
the 7,736-line descriptor/system row was divided into S40-S46 and the formerly planned
S41-S48 became S47-S54. S42-S54 remain pending after active S41. Earlier
accepted packets retain their historical prospective numbering; the linked
work plan owns current numbers.
The [CPU work packages](../etc/architecture/t539-cpu-work-packages.md)
retain the remaining CPU work as pending, not accepted CPU extraction.

## S40 Acceptance

Actual pushed NXVM P1 `e4a7615d1` has exactly nine scoped paths, passes
`git show --check`, and equals `origin/master` at actual-commit review. The
two original ARPL sources (910 lines) are retired; every base and S53 case
has a CPU or public-board receiver, including register/memory forms, illegal
encodings, protected faults and PIC IRQ delivery. Eight code/test/build/gate
paths add 689/remove 941 lines (net -252); evidence is separate. CPU tests
link only `x86-cpu`; board tests use public machine and real PIC operations.
Complete x64/x86 builds and units pass 413/413 per width; all 66 specialized
gates and the 412-row T344 matrix pass on both widths. Six unchanged Shared
manifests, documentation governance and diff checks pass. No production/API,
Shared, firmware, INI or EXE input changed. See [S40 evidence](../etc/evidence/t539-s40-arpl-migration.md).
S40 is accepted; 56 original direct-private `.c` consumers plus the common
fixture header remain assigned to S41-S51. T539 stays open.

## S39 Acceptance

Actual pushed NXVM P1 `07019f588` has exactly fourteen scoped paths, passes
`git show --check`, and equals `origin/master` at review. The 170 original
scalar IN/OUT contexts and sixty original INS/OUTS contexts retain CPU or
public-board receivers; the separate port-ownership behaviors retain their
board receiver. The thirteen code/test/build/gate paths add 1,239 and remove
1,564 lines (net -325); the 44-line evidence report is separate. There is no
production/API or EXE input change. Complete x64/x86 builds and repository-
only units pass 413/413 each. All 66 specialized gates pass per width,
including T317/T332/T337/T344, CPU/PIC authority and the 412-row direct
matrix. Six unchanged Shared manifests pass within eleven manifest tests;
documentation governance passes. See [S39 evidence](../etc/evidence/t539-s39-port-io-migration.md).
S39 is accepted; 58 direct-private `.c` consumers plus the common fixture
header remain assigned to S40-S51. T539 stays open.

## S31 Acceptance

Actual pushed NXVM P1 `38bc5b10c` has exactly nine scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 335 original
contexts retain one receiver: 324 CPU-owned and eleven board-owned. Code,
test, build and gate changes add 1,289/remove 1,468 lines (net -179); the
49-line evidence report is separate. Two CPU tests link only `x86-cpu` and
compile with warnings as errors. Complete x64/x86 units pass 397/397 each;
T317/T332/T344 and CPU/PIC gates, 396-row direct matrix, six unchanged
Shared manifests, and documentation governance pass. No production/API or
executable input changed. [S31 evidence](../etc/evidence/t539-s31-imul-group2-migration.md)
contains the original-case receiving map. The S31 packet is closed; 74
original private-test consumers remain assigned to S32-S42. S43-S45 retain
lifetime, physical relocation and whole-CPU acceptance.

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

## S32 Acceptance

Actual pushed NXVM P1 `9f785a551` has exactly eight scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 775
original contexts retain one receiver: 767 CPU-owned and eight board-owned.
Code/test/build/gate changes add 1,536/remove 1,589 lines (net -53); the
57-line P1 evidence draft is separate. Two CPU tests link only `x86-cpu` and
compile with warnings as errors; board tests use public machine operations.
Complete x64/x86 builds and units pass 399/399 on each width; all 66
specialized gates pass per width, including T317/T332/T337/T344, CPU/PIC
authority, the 398-row direct matrix, six unchanged Shared manifests and
documentation governance. No production/API or executable input changed.
[S32 evidence](../etc/evidence/t539-s32-legacy-alu-lock-migration.md) contains
the original-case receiving map. The S32 packet is closed; 72 original
private-test consumers remain assigned to S33-S42. S43-S45 retain lifetime,
physical relocation and whole-CPU acceptance.

## S33 Acceptance

Actual pushed NXVM P1 `6ca7f61ac` has exactly six scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. The
original 19 first-group functions retain one receiver each: 200 CPU-only
instruction executions and 23 public board contexts. Code/test/build/gate
changes add 1,056/remove 1,041 lines (net +15); the 43-line evidence report
is separate. The CPU test links only `x86-cpu` and compiles with warnings as
errors. The board test uses public machine operations for 11 protected
faults and twelve real-mode divide deliveries. Historic T316/T401 first-group
success markers moved to the CPU receiver. Complete x64/x86 builds and units
pass 401/401 on each width; all 66 specialized gates pass per width, including
T317/T332/T337/T344, CPU/PIC authority, the 400-row direct matrix, six
unchanged Shared manifests and documentation governance. No production/API
or executable input changed. [S33 evidence](../etc/evidence/t539-s33-inc-dec-first-group-migration.md)
contains the original-case receiving map. The S33 packet is closed; 72
original private-test consumers remain assigned to S34-S42, including the
unmigrated S34/S35 functions in `core_machine_inc_dec_smoke.c`. S43-S45 retain
lifetime, physical relocation and whole-CPU acceptance.

## S34 Acceptance

Actual pushed NXVM P1 `3b2d17d97` has exactly seven scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 13
original second-group functions retain one receiver each: 111 CPU instruction
executions and eight public board faults. Two originally mixed functions were
split at their fault sections. Code/test/build changes add 843/remove 881
lines (net -38); the 44-line evidence report is separate. One CPU test links
only `x86-cpu`; one board test uses public machine operations. The shared
test-only board-fault helper removes duplicate S33/S34 checking logic while
preserving the 23 S33 board cases. Complete x64/x86 builds and units pass
403/403 per width; all 66 specialized gates pass per width, including
T317/T332/T337/T344, CPU/PIC authority, the 402-row direct matrix, six
unchanged Shared manifests and documentation governance. No production/API
or executable input changed. [S34 evidence](../etc/evidence/t539-s34-test-add-adc-sbb-migration.md)
contains the receiving map. S34 is closed; the last 19 `inc_dec` functions
remain assigned to S35. The original private-test consumer inventory remains
at 72 until that file is retired. S43-S45 still own lifetime, physical
relocation and whole-CPU acceptance.

## S35 Acceptance

Actual pushed NXVM P1 `0fc194460` has ten scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 19
remaining functions have one receiver: 325 CPU instruction executions and
twelve public board contexts. The mixed source is retired; the CPU receiver
links only `x86-cpu`, and the board receiver uses public machine operations.
The shared test-only divide fixture removes repeated S33/S35 delivery checks
while preserving S33's cases. Code/test/build changes add 463/remove 733
lines (net -270); the 45-line evidence report is separate. Complete x64/x86
builds and units pass 404/404 per width, all 66 specialized gates pass per
width, including T317/T332/T337/T344, CPU/PIC authority, the 403-row direct
matrix, six unchanged Shared manifests and documentation governance. No
production/API or executable input changed. [S35 evidence](../etc/evidence/t539-s35-final-inc-dec-migration.md)
contains the receiving map. S35 is closed; 71 original private-test consumers
remain assigned to S36-S45. S46-S48 retain lifetime, physical relocation and
whole-CPU acceptance.

## S36 Acceptance

Actual pushed NXVM P1 `607fdea7a` contains exactly 17 scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. Four private-state FLAGS tests are retired, including the PUSHF/POPF
source includer. All original case families have one CPU or public-board
receiver; the six replacement tests pass on x64 and x86. Code/test/build/gate
changes add 1,562/remove 1,982 lines (net -420). Complete x64/x86 units pass
406/406 each; both specialized gate aggregates, T317/T332/T337/T344,
CPU/PIC authority, 405/404-row direct matrices, six unchanged Shared
manifests, and documentation governance pass. No production/API or executable
input changed. [S36 evidence](../etc/evidence/t539-s36-flags-migration.md)
records the receiving map. S36 is accepted; 66 original direct-private `.c`
test consumers plus one shared fixture header remain assigned to S37-S45.

## S37 Acceptance

Actual pushed NXVM P1 `3c826c32d` has exactly thirteen scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. The two original mixed-owner MOVS/LODS sources are retired; all 119
original contexts have a CPU or public-board receiver, with complementary
protected-fault assertions rather than duplicate production paths. Eleven
code/test/build/gate paths add 1,065/remove 1,327 lines (net -262); the
evidence and inventory documentation are separate. Both CPU receivers link
only `x86-cpu`; board receivers use public machine operations and real PIC,
guest descriptors, faults and interrupt frames. Complete x64/x86 units pass
408/408 each; both 66-target specialized gate aggregates, the 407-row T344
direct matrix, six unchanged Shared manifests, documentation governance and
diff checks pass. No production/API, Shared source/test, firmware, INI or EXE
input changed. [S37 evidence](../etc/evidence/t539-s37-string-transfer-migration.md)
records the receiving map. S37 is accepted; 64 original direct-private `.c`
test consumers plus one fixture header remain assigned to S38-S45.

## S38 Acceptance

Actual pushed NXVM P1 `ae76a7c84` has exactly fifteen scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. All 203 original STOS/SCAS/CMPS contexts retain CPU or public-board
receivers. The three mixed-owner sources are retired; six replacement tests
preserve historical success markers. Thirteen code/test/build/gate paths add
1,682/remove 2,034 lines (net -352); the two evidence documents are separate.
Complete x64/x86 builds and units pass 411/411 per width. Specialized gates
pass 66 x64 and 68 x86 targets, including T317/T332/T337/T344, CPU/PIC
authority and direct matrices of 410/409 rows. All eleven selected manifest
tests, including six unchanged Shared manifests, documentation governance and
diff checks pass. No production/API, Shared, firmware, INI or EXE input
changed; the regenerated x86 EXE build side effect was removed. The
[S38 evidence](../etc/evidence/t539-s38-string-scan-compare-migration.md)
records the receiving map. S38 is accepted; 61 direct-private `.c` test
consumers plus one fixture header remain assigned to S39-S45.

## S41 Acceptance

Actual pushed NXVM P1 `99ab4c002` contains exactly eighteen scoped paths,
passes `git show --check`, and equals `origin/master` at actual-commit review.
The 899-line mixed BOUND source is retired. Its 247-line CPU receiver links
only `x86-cpu`; its 335-line board receiver uses public machine operations,
guest table construction and the real PIC. All original families retain one
receiver: width/profile, size attributes, invalid forms, segment routes,
signed boundaries, SIB/SS, VM86, real/protected faults and IRQ delivery.
The BOUND-local predecode fixes the previously reproduced 32-bit register-only
form from internal CPU error to terminal #UD without a new path or API.

Eight code/test/build/gate paths add 609/remove 922 lines (net -313); evidence
and task state are separate. Complete x64/x86 builds and units pass 414/414
each. Both 67-target specialized gate aggregates pass per width, including
T317/T332/T337/T344, CPU/PIC authority and the 413-row direct matrix. Six
unchanged Shared manifests, documentation governance and diff checks pass.
Because CPU production changed, all four runnable profiles have rebuilt,
optimized, stripped T539 x64/x86 EXEs; their hashes and no-INI-change proof
are recorded in [S41 evidence](../etc/evidence/t539-s41-bound-migration.md).
S41 and S42 are accepted. Fifty-two direct-private `.c` consumers plus the
shared fixture header remain assigned to S43-S51. T539 remains open.

## S42 Acceptance

Coordinator actual-change review accepts pushed NXVM P4 `2935b5886`: exactly
the three named table-register smoke sources and registrations are retired;
CPU-only forms remain in their three `x86-cpu` receivers, while real guest and
board contexts remain in `machine-table-register-board-smoke`. Test sources
add 109/remove 1,370 lines (net -1,261); gate declarations add 33/remove 44.
No production, Shared, firmware, INI or executable input changed. Complete
x64/x86 repository-only units pass 415/415 per width; specialized gates pass
66/66 per width. T317 has 35 strict receivers; T332 recognizes the three
CPU-local instruction fixtures; T344 classifies the one public board
constructor (123 total). Documentation governance and diff checks pass, and
the reviewed commit equals `origin/master`. See [S42 evidence](../etc/evidence/t539-s42-table-register-migration.md).

## S43 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S43 after accepted S42 P5 `91f3ebb7d`; NXVM target only. |
| Admission And Approval | Owner's automatic-S authorization for the already approved bounded CPU-work plan; one session separates executor and coordinator review. |
| Objective | Migrate every descriptor/table/cache context from the mixed private `core_machine_descriptor_system_smoke.c` into exactly one CPU-local or public-board receiver. CPU owns instruction decode, architectural cache state, local rejection and rollback; board owns guest tables, physical memory, privilege transition, delivered fault and PIC/IRQ context. |
| Non-goals | The source's `SMSW/LMSW/CLTS/MOV CR` control-state block remains S45 and prevents source retirement in S43. No descriptor-system production rewrite, new public ABI, speculative timing claim, Shared relocation, firmware, INI, MyNES or executable-input change. LAR/LSL/VERR/VERW remain S44; debug/protected-transfer dependency groups remain S46-S49. |
| Reference Baseline | Clean pushed S42 P5 `91f3ebb7d`, x64/x86 units 415/415. [CPU work packages](../etc/architecture/t539-cpu-work-packages.md) and [inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assign the 1,080-line descriptor-system source to S43. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md), [work packages](../etc/architecture/t539-cpu-work-packages.md) and [inventory](../etc/evidence/t539-cpu-incremental-inventory.md). |
| Files And ABI Surface | The descriptor/table/cache portions of `core_machine_descriptor_system_smoke.c`, their CPU and public-board receivers, NXVM test registration/gates, test-local fixtures and S43 evidence only. The retained control-state portion is explicitly S45 input, not an S43 compatibility path. No production or Shared interface change without a revised packet. |
| Applicable Rules | Product reading set; execution, architecture, coding and documentation rules; NXVM architecture/Coding authorities. CPU-local receivers link only `x86-cpu`; board receivers use public Core operations and real guest contexts. |
| Verification | Full repository-only x64/x86 unit suites; T317/T332/T337/T344, CPU/PIC authority, direct matrix, six unchanged manifests, documentation governance and diff checks. Focused work may run the affected receivers only before the complete suites. |
| Expected Markers | Every original descriptor/table/cache/rollback case is mapped once; CPU receiver has no `core_machine` dependency; board receiver has no private CPU/RAM access. Real table and privilege effects remain public-board assertions. The named control-state block remains once in its S45 source portion. |
| Asset Needs | Repository-only inputs; retain the S18 recovery artifact. No executable rebuild unless a proven production input changes. |
| Reporting Requirements | [S43 evidence](../etc/evidence/t539-s43-descriptor-system-migration.md): original-case receiving map, source/test line counts, dual-width results, complete-P push and actual-commit acceptance. |
| Stop Conditions | A lost descriptor-system case, unresolved CPU-versus-board ownership, included-source dependency, production/API need beyond scope, or oversized indivisible receiver requires a packet revision before continuing. |
| Exit Criteria | Every S43 descriptor/table/cache context has one receiver; the source's named S45 control-state remainder is explicit and no descriptor case remains there; complete units/gates and actual-change review pass; evidence/current transfer is committed and pushed. |
| Original Owner Request | Split the oversized CPU migration into traceable S tasks with automatic admission, preserving semantics, original code style and one-owner architecture. |
| Similar-Issue Sweep | Inspect every descriptor-system instruction/form across CPU profiles, operand widths, null/invalid selectors, descriptor type/presence/busy state, CPL/VM86, table/cache/control rollback, reset and real delivered-fault/IRQ context; transfer only named out-of-scope query/control/debug groups to S44-S49. |

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
remaining 56 original direct-private `.c` test consumers and one fixture
header to S41-S51. Embedded CPU lifetime remains until S52; physical Shared
relocation is S53; whole CPU acceptance is S54. S41 owns the unresolved
32-bit BOUND observation. None is silently closed or transferred to the next T.

## Accepted Progress

| Task | Progress |
| --- | --- |
| T539 S40 | Accepted: NXVM P1 e4a7615d1 migrates ARPL ownership; original base/S53 cases retain CPU/board receivers; units 413/413 per width. No production or asset change. |
| T539 S39 | Accepted: NXVM P1 07019f588 migrates port I/O ownership; all 230 original scalar/string contexts retain CPU/board receivers; units 413/413 per width. No production or asset change. |
| T539 S38 | Accepted: NXVM P1 ae76a7c84 migrates STOS/SCAS/CMPS ownership; all 203 original contexts retain CPU/board receivers; units 411/411 per width. No production or asset change. |
| T539 S31 | Accepted: NXVM P1 38bc5b10c migrates immediate IMUL and Group-2 test ownership; all 335 original contexts retain CPU/board receivers; units 397/397 per width. No production or asset change. |
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
and verification limits. S19-S39 changed no executable inputs and require no
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
