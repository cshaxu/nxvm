# T563 S7 C17 Production Admission

## Baseline And Complete Batch

Admit from clean S6 acceptance f2cea924c. The original
[C01--C17 universe](t560-s1-cpu-convergence.md),
[post-refactor receiver census](t563-s1-rebase.md) and
[accepted C16 runtime proof](t563-s6-selected-runtime.md) remain authoritative
evidence. All original source/model restrictions survive. The selected SX is
the accepted embedded-family candidate, not every historical SX stepping or
the unverified fitted PC110 package. This batch adds no timing number or
silicon feature and does not claim PC110 qualification.

| ID | Obligation | Current path and required disposition |
| --- | --- | --- |
| A1 | Real Core construction and model identity | `machine.c` validates CPU only through 80386; `cpu_interface.h` still says instruction-only. Admit selected SX through the existing constructor, require its actual absent-FPU binding and preserve the 386-only MOV-CR quirk. Check public identity/reset/CPUID/execution, invalid inputs and atomic publication. |
| A2 | Both real Board reset routes | `machine_board.c` independently selects firmware-less high RAM alias for 286/386 and firmware-backed reset ROM alias in a later switch. Consolidate at the existing alias selector and include SX's existing 32-bit reset mapping. Prove real reset fetch/guest execution for ROM and RAM, not a host register patch masquerading as reset. |
| A3 | Actual run, source duration, preview and Debug | S6's sole aggregate and pending owner are implemented. Replace staged Core profile/max mutation with actual SX construction in their existing execution, fault, budget, purity, inspection and trace witnesses. Retain chip-only lexical/inspection scope only with an explicit reason. |
| A4 | Reset, IRQ/NMI, SMM, HALT/shutdown and absent FPU | Existing executor tests start from 386 then rebind. Construct the real selected model, retain early-family negatives separately, and exercise the existing signal/SMRAM/FPU contracts with no second mode owner. |
| A5 | DMA/HOLD/RDY, CLK and FLUSH | Both scheduler family predicates already explicitly include SX; do not broaden or replace them blindly. Reuse actual constructor in the complete existing scheduler/system witness matrix, retaining sole bus and guest-time ownership. |
| A6 | Board/adapter and PC/AT selection | `pc_at_profile.c` accepts only 8086/186/286/386; add selected SX while retaining the explicit unsupported 8088 policy. Descriptor/materialization and real Board construction must agree. Machine adapter is already generic; four App CPU choices remain fixed and unchanged. |
| A7 | Scope, source gates and receiving qualification | Remove obsolete Core-running test rebinding, audit every family predicate/caller and unchanged old CPU cohort, refresh four manifests; finish complete dual-width units/gates, selected four-profile integrations and eight artifacts before any implementation P. |

## Read-Only Census And Design

Search current source, not historical Core paths: `rg -n 'PROFILE_80|C17|486|386'
src/x86-core/machine src/x86-nxvm/board-base src/x86-nxvm/board-at
src/x86-nxvm/board-xt src/x86-nxvm/machine -g '*.c' -g '*.h'`. Follow with the
exact `PROFILE_80486SX|PROFILE_80386|PROFILE_80286` query over X86 NXVM,
X86 IBMPC and X86 Debug, and `80486|C17|unqualified.*486|production.*SX|PROFILE_80386`
over the NXVM CMake gates. The relevant production findings are A1/A2/A6.
Core's 386-only silicon option must stay exclusive to 386. Board default
resolution must stay 386. Existing scheduler SX predicates, Core selected
wait/source qualification and adapter info copy are retained real paths.
The legacy metadata gate already uses the correct minimum-386 relation.

Use one private reset-alias selector for both Board callers; no reset manager,
second ROM map or CPU-family mirror. Core is the structural CPU/FPU validation
owner. Selected SX's accepted C14 contract has no fabricated integrated or
external FPU; existing FPU arithmetic tests remain older-family tests. The
public constructor binds CPU once from the copied config. A production witness
must not later change `machine->cpu_profile`, its maximum or its CPU binding
to make an otherwise rejected model execute.

Initial C/H estimate: production +25--60/-25--65; tests +200--450/-100--200,
excluding docs/manifests/artifacts. Complete the staged-test caller inventory
before editing; refine estimates and preserve every live distinct test scope.
No public function/type/layout expansion, new CPU variant or source downgrade
is needed. The execution record below distinguishes implementation and focused
proof from the required frozen qualification. C17 and final CPU qualification
are not accepted until that complete delivery is reviewed.

## Complete Staged-Test Caller Inventory

The exact `execution_context_bind_profiles|->cpu_profile\s*=|maximum_instruction_ticks\s*=`
query over `test/x86-core/machine/*.c` finds nineteen chip-profile rebindings:
eight in executor, one observable trace, four prefetch/locality, one transaction
lifecycle, two memory inspection and three power cases. Each can consume its
actual profile at Core creation, including per-family loops. Remove all
nineteen bindings and created-machine profile overrides; no compatibility
bridge is retained. Direct chip operations inside memory-inspection tests keep
their real CPU-public contract, but obtain that CPU from SX construction.
The old-family atomic/external-wait fixture may still constrain its private
bound to exercise the old branch; selected SX must use the constructor's real
zero static bound. Preserve older-family SRESET negatives at their actual
profile owner, not by starting the SX test as a different CPU.

The existing Machine test boundary verifier can forbid future chip-profile
rebinding/created-machine profile mutation; extend its current negative
fixture cases, not a new framework or test target. This adds roughly +10--25
CMake lines outside C/H counts. A1 also consolidates the existing 386-only
MOV-CR option check into the public config validator before allocation;
remove its later duplicate admission point. SX/FPU consistency is validated
once at that Core owner, while Board retains its distinct AT capability table.
Refined C/H estimate remains production +25--60/-25--65; complete regression
and nineteen-call cleanup is expected tests +200--450/-100--200.

## Implementation And Focused Verification

Core now validates selected SX/NONE and the existing 386-only MOV-CR option
before allocation. It binds the configured CPU once through the existing
constructor. Board's existing private reset-alias selector owns both ROM-backed
and firmware-less RAM routes, including SX's high reset address; the AT
capability list now includes SX. No App changes CPU, no new interface or state
is introduced, and Lib/Emulator remain untouched.

The nineteen private profile rebindings in seven existing Machine test files
are removed. Tests now construct their actual CPU, including separate 386/SX
instances in mixed-family matrices. The Machine ownership gate rejects future
rebinding and assignment, with two negative probes in its existing test.
Old-family atomic-wait probes retain their intentional private bound; SX uses
the constructor's actual zero static bound and existing completion owner.

The new real-constructor regression first rejected SX on the old production
gate (`build/t563-s7-admission-before-x64.xml`). After implementation, identity,
neutral configuration and real ROM-alias execution pass in the three-case
`build/t563-s7-construction-x64.xml` cohort. The expanded eight-case constructor
cohort initially passes six and exposes two obsolete fixture assumptions:
the SMM test still expects unsupported on what is now a real SX, and the fault
timing test implicitly obtained cache-enabled 386 reset state by rebinding.
Keep unsupported SMI/SRESET checks at the older-family profile test; explicitly
enable cache through the public Debug CR0 operation for that cache-enabled
fault witness. Retain independent real SX reset identity and uncached entry
timing. This is not a production timing change or weakened failure assertion.
The three affected regressions then pass in
`build/t563-s7-constructor-fix-x64.xml`; the original failed checkpoint remains
recorded in `build/t563-s7-real-constructor-x64.xml`.

All source/test content and four manifests are frozen for complete dual-width
qualification. No artifacts or complete-S acceptance are claimed at this
checkpoint. S8 still owns final original C01--C17 reconciliation.

## Actual Diff And Retained Owners

Counts use `git diff --numstat` against acceptance f2cea924c, selecting C/H
separately from CMake, manifests, docs and artifacts. No new production helper,
public declaration, state, thread, clock or execution route is introduced.

| Owner | Production C/H added/deleted/net | Test C/H added/deleted/net | CMake added/deleted/net |
| --- | --- | --- | --- |
| X86 Core | 6/7/-1 | 110/153/-43 | 17/0/+17 |
| X86 NXVM | 9/8/+1 | 7/4/+3 | 0/0/0 |
| NXVM receiving gate | 0/0/0 | 0/0/0 | 5/1/+4 |
| Total | 15/15/0 | 117/157/-40 | 22/1/+21 |

The regression additions are below estimate because the existing nineteen
transport fixtures retain their matrices and now enter through the one real
constructor. Pure CPU tests still own direct chip construction; they do not
claim Core/Board admission. Retain existing source/model limits, copied
observations, the sole bus/FIFO and guest timeline, and public paused Debug
operations. The extra CMake lines enforce that distinction with two local
negative probes rather than adding a framework or cross-package test borrow.

The first receiving-gate run exposes its text slice's dependency on the old
helper position. Keep the existing private alias helper at that position and
add its ordinary forward declaration for the earlier RAM caller. Tighten the
existing controller-authority gate to identify the actual definition instead
of the declaration, and explicitly reject missing/reversed boundaries before
substring extraction. No dependency check or existing receiver assertion is
removed. This root gate belongs to NXVM, not the shared packages.

Four manifest files change sixteen hash rows in each direction, separately
from the code counts above. No filename, test target, public ABI or retained
older-family case is removed. Four owner INIs, App source/CPU choices, MyNES,
Lib/Emulator, external media/ROM and sibling repositories are unchanged.

## Frozen Qualification

Both original Ninja/ccache graphs build the complete unit aggregate and pass
their specialized gates. Direct strict-compilation verification passes
498/498 rows on each width, with zero deferred rows. Frozen repository units
pass **632/632 on x64 and 632/632 on x86**, zero failed or skipped; each includes
three automated desktop-labelled cases. JUnit is retained under
`build/t563-s7-frozen-units-{x64,x86}.xml`. The x86 build overlaps x64 units;
native test qualification itself remains serialized between architectures.

| Batch | Direct proof and retained boundary |
| --- | --- |
| A1 | Existing CPU-profile gate now uses the real public SX constructor, reset snapshot, CPUID, guest MOV-DR6 and BSWAP. Existing neutral construction tests reject out-of-range, SX/FPU and SX/386-quirk combinations atomically; old profiles retain their instruction/unsupported-signal checks. |
| A2 | Existing Board reset-ROM test executes the actual high reset fetch with SX added to its ROM matrix; the firmware-less RAM route runs both 386 and SX. Both callers use the same private selector. |
| A3 | Executor, observable trace, prefetch/locality, transaction and memory-inspection matrices obtain SX from Core construction. Existing 63 bounded-retirement cases and source/preview/pending Debug assertions survive. Pure CPU-public inspection keeps its distinct diagnostic scope without pretending to retire through Core. |
| A4 | Real-constructor FPU/WAIT, IRQ/NMI/fault, SRESET, SMM/RSM and failed SMRAM entry/restore cases pass. HALT/shutdown/reset runs both 386 and SX; older-family unsupported signals remain negative cases rather than an accidental prefix to SX setup. |
| A5 | Existing DMA grant/wait, HOLD, READY, clock/power and FLUSH fixtures construct their real model. The two already-correct SX scheduler predicates are unchanged; CPU bus and Core time remain single owners. |
| A6 | AT descriptor materialization includes SX/NONE. Adapter source inspection confirms its public CPU-profile copy has no family admission branch; construction remains Core's owner. Existing default/XT/AT/DeskPro bindings and four fixed App CPU choices are not changed. |
| A7 | Nineteen staged rebindings are removed; local boundary/negative checks, manifests, Types, dependency and strict compilation gates pass on both widths. Eight artifact and selected integration results complete the receiving delivery below; S8 remains final whole-ledger reconciliation. |

No new timing number, source-grade downgrade, hardware state mirror or fallback
CPU is part of this batch. The public header changes only the obsolete profile
comment; S6's approved observation byte is unchanged. These automated results
do not establish manual RDP, physical audio/MIDI output, complete Windows GUI
or the fitted PC110 package/stepping. Those boundaries are not waived by an
older-family boot or by selected SX construction.

## Integration And Release Delivery

Each width passes **16/16** selected integrations, with per-profile counts
Default/XT/AT/Model 40 = 3/3/5/5, zero failed or skipped. Eight per-profile
JUnit files are `build/t563-s7-<app>-<arch>-integration.xml`. Model 40 takes
120.75 seconds on x64 and 152.33 seconds on x86, within the unchanged
300-second probe budget. Each distinct scenario runs once. This is selected
boot/configuration/lifecycle coverage, not complete external-media or guest
Windows GUI qualification.

Eight current 0563 PC EXEs are rebuilt/deployed by reconfiguring each original
graph in turn, reusing unchanged shared objects and the persistent ccache.
Both graphs are restored to Default and its final artifact rebuilt. All eight
final published CLI/INI identity checks pass; their logs are
`build/t563-s7-published-<app>-<arch>.log`. Each deployed PE has the required
8664/014C machine field and no `.debug*` sections. Normal linker symbols are
allowed by the unchanged `--strip-debug` recipe. Final sizes and hashes are
in [the artifact inventory](t563-s7-artifacts.json).

The four owner INIs match their prequalification SHA-256 hashes. MyNES's pair,
App source/CPU choices and all owner media/ROM/snapshots remain unchanged.
The complete A1--A7 self-review finds no remaining admitted member or new
interface/state/owner. Source-derived model limits and S8's whole C01--C17
acceptance stay explicit. Complete target-separated implementation pushes
and coordinator actual-commit review follow; T563 is not closed here.

## Target-Separated Implementation Delivery

Shared P1 **d592300bd** is immediately pushed to origin/master with the
complete twenty source/test/manifest paths. Its +165/-188 tracked lines include
the separately reported C/H, local CMake and sixteen manifest-hash changes.
All required frozen qualification and eight published artifact checks precede
that complete delivery. Source bytes committed are the bytes already built;
the commit itself adds no executable input or timestamp-rebuild requirement.

NXVM P2 retains the full packet, receiving controller gate, evidence/index,
family history, artifact inventory and eight PC EXEs, without mixing Shared
or MyNES paths into the commit. After both complete target-separated pushes,
coordinator actual-commit review is still required. T563/S8 and the complete
CPU goal remain open.

## Coordinator Actual-Commit Acceptance

Switch to coordinator and review actual pushed Shared **d592300bd** and NXVM
**ebe095ac1**, not just their reports. Inspect the four production-file changes,
all converted/new test cases, boundary probes and four manifests, then the
receiving gate, full packet/history/index, artifact inventory and eight binary
paths. Their twenty/fourteen paths respect the target split and frozen input
boundary. Recheck both 632-case JUnit inventories, zero failures/skips, eight
committed artifact hashes and exclusion of Lib/Emulator, MyNES and App source.
The worktree is clean before this governance-only acceptance update.

Accept A1--A7 and close S7. Real construction, both reset aliases and the
existing run/preview/Debug/system/DMA receivers now qualify the selected
embedded SX without a private 386-to-SX bridge. The source/model limits,
unchanged App CPUs/owner inputs and unperformed manual verification remain
explicit. S8 and the complete original CPU goal remain open. This acceptance
changes no source, test, manifest or executable input and therefore does not
trigger a timestamp-only rebuild.
