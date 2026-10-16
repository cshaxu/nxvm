# T561 S8 NXVM Consolidation

## Admission And Baseline

Accepted input is clean `123440835`. The active packet consumes I4 from the
T561 convergence ledger. There are 152 tracked source/Core and 313 test/Core
paths remaining, including package metadata and family integration harnesses.
The S2 disposition remains the complete ownership reference; S7 already moved
neutral Machine and its tests and must not be repeated.

S8 implementation and automatic qualification are complete. This record
retains intermediate findings chronologically; the final results below
supersede their in-progress verification limits. Coordinator acceptance and
commit identities are recorded by Current after actual-change review.

## Speed Contract Consolidation

The existing Product and Machine enums describe the same standard/turbo values.
Machine owns pacing; Product only parses and displays the selected value.
The sole enum now lives in `x86-nxvm/machine/speed_interface.h`, with
`x86_nxvm_machine_speed` and unchanged standard/turbo numeric values. Both the
current Core adapter and Product descriptor consume that declaration. No old
name alias or additional runtime state is retained.

Factory callbacks retain their necessary opaque-descriptor/typed-Machine
boundary but no longer translate between identical enums. Machine status is
returned directly. In particular, an invalid enum is no longer normalized to
standard before reaching Machine's existing validation. The monitor parser
still admits only its existing standard/turbo strings; no valid user command
changes behavior. Factory tests also verify failed get/set preserves output
and prior speed while propagating the Machine error.

Both original x64/x86 Release Ninja graphs built the affected receivers with
the persistent ccache directory. Each width passed three focused regressions:
the descriptor contract, factory path and actual default Machine speed policy.
This is focused evidence, not the required final whole-repository qualification.
Build logs are retained temporarily in `build/t561-s8-speed-<width>-build.log`.
Core and X86 NXVM source/test manifests and both production corpus gates pass.

## Peripheral Chip Move

All twelve non-CPU/FPU chip leaves moved from Core into X86 NXVM: 34 source
and 42 test files. The 40 chip test registrations moved into the receiving
package; all twelve production target names now use `x86-nxvm-chip-*`.
Current source, test, CMake and family-tool path/target receivers are updated.
Stable chip symbols, numeric values, tables and algorithms are unchanged.

Comparison of all 76 moved files against `123440835`, after the single
`core/chips/` to `x86-nxvm/chips/` path substitution and line-ending comparison,
finds no other body difference. Both original graphs build the receiving
package and factory/default-speed paths. Each width passes all 47 current
X86 NXVM package tests, including 40 chip cases and package checks. Added
negative probes reject chip-to-Product and undeclared chip-to-chip links.
The corpus checker validates the existing Types-only chip construction helper
before checking extra link edges; it does not silently skip arbitrary helpers.

Temporary build/test logs use `build/t561-s8-chips-<width>-{build,tests}.log`.
No artifact rebuild or complete repository qualification is claimed yet.

## Board, Machine And Product Structural Pass

The structural pass moved 382 remaining tracked paths into X86 NXVM and
updated 854 owner identifiers and their receivers. Product is now the single
implementation target rather than an interface forwarding to retired Core.
The original x64 graph completed all 448 build steps for the consolidated
test target. This is build evidence only; the consolidated tests have not
yet received their required dual-width qualification.

The merged dependency-negative test now retains both source-edge and
build-edge rejection cases. It rejects chip-to-Machine source dependencies,
noncanonical outer includes, chip-to-Product links, undeclared chip peers
and Product-to-Console links. A direct CMake run passed all five probes.
Manifest and corpus registrations now pass the receiving checkers' actual
root parameters. The source and test manifests were refreshed after the
public-Machine PIC fixture migration below; further edits require refresh.

## Board PIC Tests Use Public Machine Execution

All five formerly raw-CPU Board/PIC cases now execute through the public
Machine and read copied CPU snapshots. Guest LGDT/LIDT/LMSW, segment loads,
LTR and IRET establish protected state rather than writing hidden CPU caches.
The executing Machine also owns the Board PIC ports; the old separate raw
CPU plus port-only Machine arrangement is gone.

Preserved predicates by case:

- Protected data: full EAX value after DS load, CS/IP, saved return IP,
  exact SP, halted handler and PIC IRR/ISR.
- Protected far jump: handler CS/IP, saved target IP, halted handler and
  PIC IRR/ISR.
- IDT privilege entry: CPL 3 established by IRET, kernel CS/DPL/SS, exact
  post-interrupt SP, cleared IF, halted handler and PIC IRR/ISR.
- Outer IRET: 32-bit return through a 16-bit stack with nonzero high ESP,
  subsequent IRQ delivery, handler CS/SS/IP, halt and PIC IRR/ISR.
- 16-bit task switch: guest LTR then task JMP, pending IRQ delivered after
  the destination task enables IF, handler CS/IP, halt and PIC IRR/ISR.
  The final TR additionally proves that the target task was entered.

Five private CPU fixture headers were deleted. One 103-line Board-local
fixture replaces the two data/far cases; the other three reuse the existing
Board protected bootstrap fixture. No CPU/Machine production API, state or
algorithm changed. Relative to the moved bodies, these five tests and six
fixture paths total +330/-1111 (net -781); this excludes unrelated migration
changes and the one obsolete composition-source link removed from CMake.

Both original cached graphs rebuilt the five final targets and passed 11/11
focused checks each: five executable cases plus Types, test ownership,
source/test manifests, corpus and verifier-negative checks. Temporary logs
are `build/t561-s8-public-pic-<width>-{build,tests}.log`. These are not the
complete repository, profile integration, artifact or manual desktop gates.

## Remaining I4 Work After Structural Pass

Repository-dependent checks are now registered by
`cmake/nxvm/NxvmRepositoryTests.cmake`, with six scripts in its `tests/`
directory. The five manual-ledger runners and three result checks are
registered by `NxvmTimingTests.cmake`; their Board qualification source remains
with its owner. The independent component entry no longer requires NXVM
manuals, PowerShell catalog generation or concrete App checkers. This is a
registration split, not deletion of the repository's qualification cases.

The negative tests exposed two stale migration receivers: the DAG classifier
matched the entire X86 NXVM package before its child owners, and the CPU/PIC
checker recognized only the retired PIC prefix. Both were corrected; real
source DAG and both negative tests now pass. The route-owner gate and its
negative probes use current package labels rather than retired Core labels.

Both original graphs rebuilt the separated targets and passed all 229
X86 NXVM-labeled tests per width, including the relocated repository checks.
Logs use `build/t561-s8-registration-<width>-*.log`. This does not qualify
unselected packages, all Apps, integrations or renewed artifacts.

- Review moved board/adapter/Product bodies and all receivers against the
  frozen baseline; remove remaining retired naming without aliases.
- Verify independent package configuration and retain family setup and concrete
  App integration ownership after the registration split.
- Reconcile current design documents, frozen path/symbol ledger and manifests;
  run the complete packet verification and renew eight PC artifacts before P.

Lib, Emulator, CPU/FPU implementation, MyNES and owner INI/ROM/media/snapshot
inputs remain unchanged. S9 retains independent-package/final T acceptance;
the CPU qualification successor remains deferred.

## Canonical Symbol And Test-Path Pass

The next mechanical pass renamed 152 remaining production symbols across 83
tracked receivers. Keyboard/mouse mapping and PC/AT preparation belong to
Machine; AT wiring, descriptors, ROM and profile contracts belong to Board;
version/copyright belong to Product. Hardware chip prefixes and all values,
tables and execution algorithms remain unchanged. No compatibility aliases
were added. Ninety Board test paths lost the retired `core_` prefix; exact
filename/stem references and computed CMake source paths were updated together.

The x64 full unit-target build completed after correcting computed filename
receivers. Source/test manifests and the production dependency checker pass.
Full repository units are running; the first discovered stale root test
boundary probe expected X86 NXVM's legitimate public Debug dependency to fail.
It now admits that edge and explicitly rejects Core-to-NXVM and NXVM-to-App
edges. The retired `core` test-layer alias was removed, and the negative
selftest passes. This root infrastructure receiver belongs in the separate
Shared-target commit, not a Lib or Emulator implementation change.

Current long-running evidence uses `build/t561-s8-naming-<width>-build.log`
and `build/t561-s8-naming-<width>-units.log`. The x86 run passed 626/626;
the x64 run passed 625/626 and its corrected boundary selftest then passed.
The subsequent final-target-name x64 run passed all 626/626 in one execution;
the corresponding x86 final-name run also passed 626/626. Both final-name
unit-target builds completed in the original Ninja/ccache graphs. Four-profile
integration, specialized gates and eight-artifact renewal are now running;
their success is not inferred from these unit results.

The count reduction from 632 to 626 consolidates the six duplicated package
gates: Types, ownership, source manifest, test manifest, corpus and negative
verifier. Old Core and X86 NXVM each registered these before consolidation;
the combined package owns each once. Behavioral tests and the separately
registered repository checks remain present.

Normalized baseline comparison covers 108 moved production C/H paths:
102 match exactly after the recorded path and symbol substitutions. The six
remaining files are Machine's machine.c, machine_interface.h, machine_private.h,
runner.c and waiting.c plus Product factory.c. Their residual differences are
the single Machine-owned speed enum and removal of Product's duplicate enum
conversion. The factory now preserves invalid speed/status semantics rather
than silently coercing an unknown value to standard. No execution/timing
algorithm changed. The review table and normalized comparisons are retained
temporarily under build/t561-s8-production-review.csv and its normalized files;
the tracked [production review](t561-s8-production-review.csv) now records
these 108 paths plus the 34 chip C/H moves. All 34 chip bodies match the
baseline after include-path replacement. The complete delivery still requires
the receiver/test/artifact actual-change ledger.

Retired Core-named Board targets and their root build-inventory references
now use x86-nxvm-board names. These name changes do not alter test predicates,
budgets or labels. Current architecture/layout/source-policy descriptions now
identify the real family owners. The NXVM documentation governance check passes.

The x64 four-profile integrations passed. The following specialized gate
found a stale constructor regex: it recognized the neutral Machine constructor
only through an old substring and omitted renamed Board construction. The
checker now names the two real public entry points explicitly; the original
140 classified sources pass unchanged. It also includes the newly public
PIC/data/far fixture, increasing the checked inventory to 141, rather than
lowering a count to conceal missing constructors. Qualification resumes at
the x64 specialized-gate/artifact stage without rerunning the completed boots.

## Final Delivery Evidence

Both original Release Ninja/ccache graphs built the final unit targets and
passed 626/626 repository units. The two root boundary probes were then
updated to use real current package names and passed on both widths.
Each width passed all four profile integrations: default 35 (including 12
MyNES regressions), XT 3, AT 5 and DeskPro 5, totaling 48 executions with no
skips. Final restored-default identity and actual published CLI/INI checks
passed. The two family INI/identity integrations now carry x86-nxvm labels;
the x64 final-label replay passes both cases. Both specialized-gate builds
pass, including constructor classification, dependency, documentation,
manifest, exact direct-compilation and negative-verifier checks.

The specialized sweep repaired old paths/dynamic symbols in the executor,
controller and mutable-state inventories. The consolidated Product already
compiles under package strict options: factory/ini/startup are therefore
removed from the obsolete deferred-source list, not exempted from checking.
The direct matrix verifies all 495 listed commands as retained strict. This
does not remove or redefine historical CPU warning exceptions outside S8.

All eight 0561 PC EXEs were rebuilt and deployed in their existing asset
directories. [Artifact records](t561-s8-artifacts.json) contain relative
paths, PE machine values, sizes and SHA256 hashes. Actual published processes
use each App's adjacent INI name; version and two-artifact count checks pass.
INI contents, ROM/media/snapshot inputs and MyNES binaries are unchanged.
No Lib, Emulator, X86 Debug, X86 IBMPC or X86 Core source/test file changed.
Only the root build ordering and root test-boundary infrastructure require
a separate Shared receiver commit.

The [file review](t561-s8-file-review.tsv) records the complete non-artifact
delivery surface; the separate production table distinguishes pure moves.
Git's rename-aware C/H diff is production +7,034/-7,039 (net -5), test
+9,097/-9,868 (net -771). These totals include mechanical path/symbol changes;
they are not new algorithm counts. Five PIC tests retain their guest/frame/
register/PIC predicates through public Machine execution, removing five
private CPU fixture headers. The only new fixture is the 103-line shared
Board/PIC fixture; the new speed header replaces duplicate enum declarations.
No runtime state, thread, queue, scheduler or observation API was added.

Manual physical-desktop/RDP and fresh diagnostic-ROM runs are not claimed.
S9 still owns independent-package/final T qualification. CPU C16/C17/final
qualification remains in its successor, not completed by this refactor.
