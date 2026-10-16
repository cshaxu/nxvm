# T561 S9 Final Package Qualification

## Baseline And Scope

Baseline is accepted, pushed `0349e2f5c`. S9 consumes I5 and reconciles I0--I4
against the complete S2 679-path inventory. This is final structural and
receiving qualification, not completion of the deferred 486SX C16/C17 work.
The original root Ninja graphs and persistent project ccache remain in use.
Separate `build/standalone-<package>-<width>` graphs exercise the actual package
test entry points; they are not replacement product build directories.

Architecture-governance review precedes coding-governance review: public inward
contracts and one production owner are checked before naming/size judgments.
No production change or new public contract is currently required.

## Confirmed Test Gate Repair

The root test-boundary checker already rejects X86 Core includes from X86 Debug
and X86 IBM PC, but its explicit link patterns omitted `x86-core-*` for both
owners. Its foreign-test-package source pattern also omitted `x86-core`.
These are checker omissions, not evidence of a production reverse dependency.

Added self-test cases first reproduced `Missing boundary rejection Outward`
for an X86 Debug link to `x86-core-machine`. The correction adds the missing
package to the existing patterns, with no helper, new state or allowlist bypass.
Six additional assertions cover both forbidden links, a legal X86 NXVM to
X86 Core link, and foreign test sources in both directions, including the
list-directory form. The complete
self-test now passes. Production source remains unchanged.

## Non-PC Consumer Review

`test/x86-core/machine/non_pc_board_smoke.c`, not the white-box
`neutral_link.c` owner test, is the independent public consumer. It includes
only Lib Types and Machine public interfaces and links `x86-core-machine`.
It executes project-authored reset ROM bytes, checks RAM/MMIO writes and ROM
write protection, an alias, an unmapped query, side-effect-free observation,
HALT with a seven-tick device deadline, interrupt acknowledgement/entry/IRET,
bounded run boundaries, reset and exactly-once finalization. No PC chip,
board, product, App or private fixture is used by this consumer.

## Independent Package Results

Both architectures configure and build from each package's own test entry,
without a repository-root configure: X86 Core passes 165/165 and X86 NXVM
passes 215/215 on each width. The CPU and FPU targets are independently
selectable and build successfully on both widths; CPU retains its actual FPU
link dependency, not a fictitious optional implementation. Package manifests,
negative gates and owner checks run within these suites. Logs use
`build/t561-s9-<package>-<width>-*.log`.

Ninja's actual non-PC executable link inputs are its one test object and
`libx86-core-machine.a`, `libx86-core-cpu.a`, `libx86-core-fpu.a` only. Types is
an interface dependency; no PC library is linked. Machine's observable target
uses the same source list with the explicit trace build capability, not a
second execution implementation.

X86 NXVM's standalone 215 versus root-label 229 cases is deliberate ownership,
not a skipped subset: six repository integration-boundary checks and eight
manual-ledger runner/result checks are registered by repository CMake. They
run in the full root unit suite. The standalone compile-command graph has no
App source/test input; X86 Core's graph has no Emulator, Debug, IBM PC, NXVM
or App source/test input. All four standalone graphs use the same persistent
`build/.ccache` launcher as the original product graphs.
All 71 public `*_interface.h` headers in the two source packages also compile
individually under C11/strict warnings on x64 and x86. The temporary probe
contains a harmless declaration so macro-only headers do not produce an empty
translation unit; no production fix was required.

## Complete Frozen Inventory Disposition

The [679-row disposition](t561-s9-disposition.tsv) records original/planned/
actual paths and original/current SHA-256 values. Its baseline is the original
S2 inventory, not a newly narrowed universe. All listed destinations exist;
an empty destination means an explicitly removed fixture/facade, not a lost
implementation. The prior S3--S8 actual-change evidence supplies semantic and
assertion preservation, which filename existence alone cannot prove.

Of the rows, 539 retain the planned path, 90 use canonical Board test names,
three use canonical CPU runner names and six put CPU gates at their actual
CPU owner. The remaining 41 are explicitly classified: seven neutral test
owner corrections; two Board timing-ledger runners; six repository checkers;
three copied-construction fixtures replaced by the real Board construction
assay; five private CPU fixtures replaced by public Machine/PIC setup; four
private controller/time fixtures replaced by public receivers; two dead boot
shims; two mirrored registration fixtures; one unused scalar facade; and nine
build/manifest/checker partitions or consolidations.

Removed fixture proofs are in S7's public registration, KBC/XT PPI, real
construction, time/deadline and final receiver sections, and S8's five PIC
predicate mappings. The actual construction assay is
`test/x86-nxvm/board-base/board_binding_fixture.c`, which includes the real
same-owner `machine_board.c` and injects its dependency failures; no copied
constructor remains. The unused scalar facade had no continuing consumer.

## Final Boundary Sweep

`git ls-files src/core test/core` returns no files. Searching current
source/test/build/tool receivers for retired root includes finds only MyNES's
own `app-mynes/core` includes, which are unrelated and remain unchanged.
No outer PC source/test includes a private X86 Core CPU/FPU/Machine header.
No retired Core production target or forwarding alias remains. Root verifier
and historical test names containing the generic word Core are not duplicate
production targets. CPU/FPU source has no diff from accepted S3 `8bb5bcaa5`.
The two retained `vm_app_machine` / `vm_app_firmware` data symbols are the
existing build-selected App binding and generated immutable firmware input,
one definition per executable. They are not retired Core APIs or forwarding
aliases. Their types and consumers use canonical X86 NXVM contracts; the
firmware embed recipe emits the same App-owned link symbol. No neutral
component depends on either symbol. Unrelated App-symbol renaming is not
required to establish the extracted package boundary.

The broader test sweep also finds existing App tests including the NXVM
adapter's `x86-nxvm/machine/machine_private.h`. No production App includes it.
This is distinct from X86 Core's private CPU/FPU/execution layouts, whose outer
borrowing is removed by S7/S8. Standalone package builds have no App input and
do not use this coupling to prove independence. The App-specific adapter
encapsulation review is explicitly recorded in TODO with owner, risk and
admission constraints; S9 does not claim all App fixtures are black-box tests
or expand this structural extraction into a new adapter observation API.

The separate 486SX successor retains C16's complete runtime/preview/timing
obligation, C17's closed production-admission gate and final CPU qualification.
The refactor neither widens that gate nor promotes evidence grades.
The CPU instruction translation unit retains its documented narrow compiler
warning exceptions. A passing strict-compilation inventory is not a claim
that every source is compiled without any historical exception.

## Final Requirement-To-Proof Audit

| Requirement | Current direct proof and remaining boundary |
| --- | --- |
| I0 exact prerequisite import | S1's pinned 264-file import review remains preserved. Lib, Emulator, X86 Debug and X86 IBM PC source/test trees have no diff from S1 `15d86fc8e`; X86 NXVM is intentionally expanded by the admitted consolidation. |
| I1 complete original inventory | All 679 original rows have explicit final destinations or reviewed removal/consolidation dispositions and current destination hashes in the linked ledger. |
| I2 CPU/FPU independence | Source/test owners and independent targets exist; no source change from accepted S3; both independent test builds pass. CPU's real FPU dependency remains explicit. |
| I3 neutral Machine | Public-only real CPU non-PC consumer passes on both widths; actual link inputs contain only Machine/CPU/FPU. S4--S7 contract reviews retain Board policy outside the neutral executor. |
| I4 NXVM consolidation | Every original remaining path is accounted for; four App machine targets link X86 NXVM Machine/Board plus the sole X86 Core Machine. No old root or outer private CPU/Machine include remains. |
| I5 final qualification | Both independent suites, 626 root units, 48 profile integration executions and final specialized gates pass on each width; eight published CLI/INI/PE/version/hash checks pass. |
| No altered CPU qualification claim | CPU successor retains the full original C16/C17/final ledger and closed production gate; no algorithm/grade or selected App CPU change. |
| Source/test hierarchy | CPU/FPU/Machine tests follow their owners; chip leaves, Board families, Machine and Product tests follow X86 NXVM. Cross-App/manual-ledger orchestration stays in repository CMake rather than independent packages. |
| Preserved assets and receivers | Eight PC EXEs rebuilt and deployed as 0561; final hashes are recorded in the artifact ledger. INI/media/ROM/snapshots and MyNES remain unchanged. |

Both original root graphs pass 626/626 repository units. After adding the
list-directory negative spelling, the exact changed boundary self-test also
passes again on both widths. No unit case or assertion was weakened. Root
suite logs are `build/t561-s9-<width>-root-units.log`.

The S9 implementation delta is confined to the existing root test checker and
self-test: +10/-4 lines, net +6, two files. Production is +0/-0. No package
contents changed, so no package manifest regeneration is appropriate. The
documentation ledger has 679 data rows; generated evidence is counted
separately from implementation. Queue/TODO current-owner wording is corrected
without editing historical evidence or shared rules.

Receiving integrations pass on both widths: default 35, XT 3, AT 5 and
DeskPro 5 executions per width, including the default graph's MyNES receiving
cases. DeskPro completes in 130.19 seconds on x64 and 169.46 on x86, without
altering the 300-second budget. Both graphs are restored to default and pass
the restored INI identity test. Final specialized gates pass, including the
495-row direct-compilation inventory. All eight published executables pass
real CLI/INI identity, PE width, version and release-count checks; the
[artifact ledger](t561-s9-artifacts.json) records their final hashes.
The final artifact ledger is produced only after all script phases succeed.
No failed or skipped integration is counted as passed. Delivery is ready for
coordinator actual-change review; acceptance is recorded separately.

Physical desktop/RDP and fresh external diagnostic-ROM executions are not
claimed by package unit results. Owner INIs, ROM originals, media, snapshots
and MyNES artifacts remain outside this change surface.
