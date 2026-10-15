# T561 S3 CPU/FPU Extraction Review

## Scope And Frozen Input

Baseline is accepted S2 `1ecd72846`. This batch consumes I2, not I3--I5 or
T560's C16/C17/final CPU qualification. No sibling source is read or imported.
Lib, Emulator, X86 Debug, X86 IBM PC, the existing X86 NXVM contract and MyNES
source/test/assets remain byte-identical to that baseline.

The architecture-governance skill fixes sole ownership and the real CPU-to-FPU
dependency before coding-governance applies namespace/path changes. No new
state, worker, scheduler, wrapper or compatibility alias is introduced.

The owner additionally requires source/test component hierarchy symmetry.
`src/x86-core/cpu` and `fpu` correspond to `test/x86-core/cpu` and `fpu`;
all six CPU-specific CMake checkers reside in the CPU test directory, not the
package root. Only package registration, manifests and package-boundary
checks remain at the root. Machine receives its own matching directory in
its later admitted batch; no empty placeholder is created now.

## Complete Changed-File Review

[File review](file-review.tsv) records old/new path, both SHA-256 values,
rename-aware added/deleted lines and classification for every changed file
under src/test/cmake/tools, excluding self-generated manifests. It includes
the unchanged-content FPU header move. [Symbol map](symbol-map.tsv) records
all 511 CPU-owned declaration names and their canonical replacements.
These are source identifiers, not 511 new public APIs.

The original 118 CPU/FPU paths become 12 implementation/header files and
106 test files. S2's total was correct, but its 11/107 split missed
`cpu_trace.h`; this is a count correction, not added scope. Nine CPU and
three FPU source/header files all match their original contents exactly after
the declared symbol/include-path replacement. No instruction table, algorithm,
timing rule, model enum value, struct layout or existing warning exception
changes. Original CPU/FPU file formatting is preserved.

All changed C/H receivers likewise match the same mechanical replacement,
apart from two explicit test-only dependency removals:

- `cpu_protected_iret_state_smoke.c`: use existing object addresses rather than
  Core's reference macro; remove the unrelated Core utility include.
- `cpu_vm86_iret_state_smoke.c`: test the existing bit mask directly rather
  than Core's utility macro; remove the same unrelated include.

Both preserve their assertions. Three CPU decoder runner filenames lose the
old Core prefix. Six checkers retain their original assertions/probe behavior
and change only the owned CPU source/fixture path. Old source/test CPU/FPU
directories and `core-chip-cpu`, `core-chip-fpu`, `x86-cpu` aliases are retired.
Similarly named board-owned functions, including refresh-pulse handling,
are not renamed merely because they share the old CPU prefix.

## Infrastructure And Distinct Retained Paths

`x86-core-cpu` and `x86-core-fpu` are actual libraries. CPU depends on FPU's
public contract and Lib Types; FPU depends on Lib Types. The no-FPU selection
preserves the original absence behavior; extraction does not implement a new
x87 model. The package and its tests have independent CMake entry points.
Package source/test gates reject outward includes, raw CRT/platform vocabulary,
outward target links and incomplete/noncanonical manifests. The manifest
checker uses the existing canonical format and algorithm, with package-local
diagnostics. There is no old-format fallback or generator revision in its hash
contract. A small negative-test helper owns only repeated rejection assertions.

Core/App/tool receivers consume the new declarations and real targets.
Repository CPU fixture/strict-compilation/qualification gates are relocated or
repointed, not disabled. Four historical evidence checks explicitly pair the
new live symbol with its unchanged historical spelling; evidence itself and
timing grades are not rewritten to look newly qualified.

The old decoder and inventory runs have distinct existing output contracts:
package-local decoder JSON and repository timing-ledger JSON. Their six target
registrations are retained. PC/board timing-manifest runners remain with Core,
because their qualification depends on the actual board composition. Neither
becomes an independent CPU test through a misleading move.

Root aggregate registration now recognizes both Core and X86 Core's canonical
component tests. It must not register an extra `unit.*` alias for a moved case.
The existing registration verifier checks both generated CTest files. This
preserves all original assertions and strict CPU fixture checks, without the
40 accidental aliases exposed by the first extraction configure.

The complete test-receiver sweep also records four existing white-box input
owners that this CPU-only batch does not claim to finish: the three
`test/core/x86/core_machine_{system_power,memory_inspection,executor_run}_smoke.c`
tests and `test/core/board-base/chips/cpu/support/cpu_instruction_fixture.h`
still include CPU-private headers. The execution tests receive explicit review
with their S5 Machine extraction; the PC/PIC fixture family receives S6 board
test consolidation and must use the public CPU contract rather than import
lower private layout. No production receiver includes those private headers.
These are explicit I3/I4 receiving obligations, not qualification of the final
package boundary or a permanent test-only exception. Independent CPU/FPU tests
already need neither Core nor a PC fixture. Final I5 acceptance must reject any
unresolved higher-package private-header borrowing.

## Actual Line Accounting

Method: compare accepted baseline bytes against final bytes, pair all 124
moved paths explicitly (118 C/H plus six checkers), and count line operations
with `difflib.SequenceMatcher(autojunk=False)`. CRLF/LF normalization is not
counted as changed text. Documentation, binary artifacts and manifest rows are
excluded. Mechanical identifier replacements count as one removed and one
added line; the map permits separating them from structural changes.

| Surface | Files | Added | Deleted | Net |
| --- | ---: | ---: | ---: | ---: |
| Production C/H | 52 | 4,668 | 4,668 | 0 |
| Test C/H | 341 | 10,066 | 10,069 | -3 |
| Source package including build/checkers/README | 58 | 4,828 | 4,684 | +144 |
| Test package including registration/checkers | 354 | 10,298 | 10,245 | +53 |
| NXVM CMake integration/checkers | 23 | 274 | 260 | +14 |
| NXVM tools | 3 | 16 | 13 | +3 |

The positive infrastructure delta establishes a genuinely independent package
and checks its forbidden edges; it is not another CPU implementation. The root
CMake registration adds two lines. No other production C/H change is hidden
in those infrastructure totals.

## Qualification

Both original Release Ninja/ccache graphs pass their final complete run:

| Verification | x64 | x86 |
| --- | --- | --- |
| Complete repository units | 627/627, 57.80 s | 627/627, 70.86 s |
| X86 Core owner cases, included above | 112/112 | 112/112 |
| Existing specialized-gate aggregate | Passed | Passed |
| Default profile integrations, including MyNES receivers | 35/35 | 35/35 |
| My5160 profile integrations | 3/3 | 3/3 |
| My5170 profile integrations | 5/5 | 5/5 |
| DeskPro profile integrations | 5/5 | 5/5 |
| Restored-default identity | 1/1 | 1/1 |
| Published EXE CLI/INI receivers | 4/4 | 4/4 |

Each profile's integrations run once. The recorded 48 integration executions
per width include the default profile's MyNES cases, not 48 different PC
models. The DeskPro group takes 132.02 s / 169.95 s; its existing assertions
and 300-second startup budget are unchanged. Automated desktop-labelled units
and integrations pass. Initial stale-path/historical-spelling gate failures
and the duplicate-registration intermediate configure are diagnostic attempts,
not the final pass.

Original graphs are restored to the default profile. No alternative graph,
cache reset, source-specific timing downgrade, host setting, timeout or
assertion weakening is used. The persistent cache and `-j8` build / `-j4` test
policies are retained. Final direct-compilation evidence retains its existing
510 strict rows and three explicitly deferred rows, not a no-exception claim.
The original instruction-file warning exceptions also remain explicit.

Ignored local logs are `build/t561-s3-<width>-<profile>-integration.log`,
`build/t561-s3-<width>-final-{units,gates,restored-identity}.log` and the eight
`build/t561-s3-<width>-<app>-published-cli.log` files. The published receiver
executes INFO, SPEED turbo/standard, HELP and EXIT, including wrong-INI negative
checks, against every actual Assets EXE rather than merely a build-tree copy.
Current source/test Core and X86 Core manifests, package boundaries, dependency
DAG, Types checks, documentation governance and `git diff --check` pass.
All 438 file-review hashes match the delivered bytes.

Eight optimized, stripped 0.5.0561 PC EXEs are deployed. Native deployment
checks verify product identity and optimized/debug-section policy; the final
receiver independently checks PE architecture, version, exactly two EXEs per
PC Assets directory and the following SHA-256 values:

| Assets file | PE | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| nxvm/nxvm_0_5_0561_x64.exe | 8664 | 1,513,079 | C975857EBCC826B7550BC94254E332953DE42B2E7768370D896FD6EF560057FB |
| nxvm/nxvm_0_5_0561_x86.exe | 014C | 1,658,273 | 5EF25B95356932D60EAC789A955CD123EA0008D015A6A909FA32AFD5FB750C78 |
| my5160/my5160_0_5_0561_x64.exe | 8664 | 1,502,057 | A4AA068A3E9CACB7BE4C96A0D40C32D494554E06E4C47BDD6850F111F5391377 |
| my5160/my5160_0_5_0561_x86.exe | 014C | 1,645,720 | 6293D35560B663081D475B4A2B8E948809ACEA47EDF05A0D94CD0C96A5287A22 |
| my5170/my5170_0_5_0561_x64.exe | 8664 | 1,513,909 | 378F47F6014FE9767AB842EB080AC697546E1DD4659697A930BF006396384C8E |
| my5170/my5170_0_5_0561_x86.exe | 014C | 1,659,487 | B2AC71CF977810B9026FF801699C5C56BE68CDF5F5470FF49AA73C18B604313D |
| mydeskpro386/mydeskpro386_0_5_0561_x64.exe | 8664 | 1,501,149 | B8AD287452113BB6C9922FE5ED5F70E3524F242CE09FD2439922F6D35FBFD7FC |
| mydeskpro386/mydeskpro386_0_5_0561_x86.exe | 014C | 1,646,222 | F553BB5D31402308249D50E3D83E54D63DB9009EA388F0ADB6D1AC5D6202DA25 |

The five imported source/test pairs, MyNES source/test/assets and every owner
INI compare unchanged to `1ecd72846`. External firmware, disks and snapshots
are neither edited nor imported. The updated PC EXEs embed the same already
approved BYOB inputs through the existing build route.

Manual RDP input/DPI, physical machine behavior, full interactive Windows GUI,
PC110, V30 and production 486SX admission are not claimed. This extraction
preserves the existing qualified/compatibility/unallocated distinctions.

## Publication And Remaining Work

Shared publication and NXVM receiver retirement use separate sequential commits
inside S3. Shared `8bb5bcaa5` adds the independently buildable new package; the next
NXVM commit switches the receivers and removes old implementations/tests.
The bounded publication interval is not a permanent alternate runtime path.
Coordinator acceptance reviews the complete cutover and only then discharges
I2. I3--I5, planned S4--S7 and the original CPU successor remain open.
