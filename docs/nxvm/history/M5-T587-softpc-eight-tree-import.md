# M5 T587 S3 - SoftPC eight-tree import

Owner-approved scope: import the committed Lib, Emulator, X86 Debug and
X86 IBMPC source/test trees unchanged from SoftPC
`dfe955eb885e66c5549b6cd7823bb262af83ab69`, then qualify all fourteen
receiving executables. NXVM baseline is `36643e553`.

This is project-owned MIT shared code, with existing notices preserved.
No independent third-party source, firmware or media is imported.

## Design and admission review

The only C production delta corrects Console metadata endpoint conversion
and rejects lost cooked-buffer capacity rather than pretending that regrowth
restores discarded content. The existing new-frame surface preparation remains
its sole capacity owner. No public API, new state, retry or alternative path.

The owner explicitly accepts the upstream Built on refresh: only composition.c
bypasses ccache and recompiles on each dependent build invocation. Other objects
retain their cache. This intentionally supersedes the S2 no-change compile/link
observation for this object and its downstream links. Sequential product build
invocations can refresh this object repeatedly; no second implementation is added.

Two checks gain RUN_SERIAL without timeout growth. Four X86 source/test trees
are already byte-identical. Planned delta including manifests: 11 files,
+46/-43 (net +3); excluding four manifests: +39/-36 (net +3).

## Verification and delivery

All admitted build, publication, test and actual-diff checks are complete.
S3 is delivered pending owner acceptance; T587 remains open.

## Actual-change review

Compared each changed source, build rule, test and manifest against the approved
upstream delta. All 296 files match the pinned commit byte-for-byte; manifest
headers, sorted unique paths, complete file sets, LF bytes and SHA-256 entries
match. X86 Debug/IBMPC source and tests have no change. Public headers, layouts,
App code, X86 Core/NXVM and configuration remain unchanged.

The Console helper retains original-target final viewport calculation, cursor
restoration and required-step failure propagation. Its sole native metadata
setter converts the right/bottom endpoint convention; ordinary window-setting
calls are unchanged. Lost cooked cells are rejected, whereas new frames retain
the existing invalidation and ensure-surface route. The changed fixtures retain
palette-unsupported fallback, viewport failure, cursor/content restoration and
new-frame capacity checks; no second renderer or state is introduced.

The new Emulator object target belongs to Product, inherits strict compiler
options and links only its existing inward dependencies. Target-owner parsing
now permits a suffix under the same declared component; it does not permit an
App dependency. Built on refresh is the owner-approved single-object cache
exception, including repeated refresh during sequential product builds.

Static gates pass for Types/dependencies, KVM naming, Emulator/X86 corpora and
all three product documentation structures. Two initial standalone invocations
omitted required root arguments: the dependency invocation was stopped and the
naming invocation rejected its missing root. Correct invocations passed; those
launch mistakes are not source-test failures and no gate was weakened.

Code/build/test accounting (git diff --numstat, excluding manifests, docs and
binaries): production C +7/-11; test C +17/-21; CMake +15/-4; total +39/-36,
net +3. Four manifest files contribute +7/-7. There is no new public API,
production state, retry loop or recovery path.

## Cached full qualification

The existing `tools/build.cmd all --no-pause` invocation configured and
published all seven x86/x64 products in 247.03 seconds. It used the existing
`build/.ninja/gcc-{x86,x64}-release` trees and repository ccache. Its 16 C compile
steps comprise fourteen intentionally uncached composition timestamp updates
and the two changed Console objects; unchanged production objects were reused.

Both standard receiving graphs then used `cmake --build ... --parallel 8`,
followed by CTest `-L ^unit$ --parallel 4` and `-L ^integration$ --parallel 1`
with output-on-failure and JUnit results. No source, assertion or timeout was
changed during qualification. These are cached builds and full registered
suites, not fresh compilation of every unchanged source or a claim of zero
legacy compiler exemptions.

| Architecture / suite | Passed | Failed / skipped | Elapsed seconds |
| --- | ---: | --- | ---: |
| x86 unit | 716 | 0 / 0 | 295.82 |
| x86 integration | 48 | 0 / 0 | 300.77 |
| x64 unit | 716 | 0 / 0 | 482.96 |
| x64 integration | 48 | 0 / 0 | 229.93 |

Native Window/Console and MIDI cases ran inside these integration suites and
passed on both architectures. This is real native API execution through the
existing automated fixtures; manually operating the desktop/RDP keyboard,
listening to audio, Linux execution and MSVC qualification are not claimed.
The Console unit fixture separately proves injected failure behavior and
endpoint arguments; it is not substituted for the native integration result.

The standard graph exposes only the selected PC profile's boot/Console rows.
Additional profile configurations therefore use the same build trees and test
new/changed registered commands and executable hashes; unchanged already-passed
cases are not rerun. Final profile and optional MyRaidenII results follow below.

## Profile completion and final publication

| Profile | x86 passing additional checks | x64 passing additional checks |
| --- | ---: | ---: |
| My5160 | 20 | 20 |
| My5170 | 22 | 22 |
| MyDeskPro386 | 23 | 23 |

All 130 checks passed without retry or skip. This includes each selected
profile's real boot, deployed INI identity and applicable Console/CMOS rows.
The standard dual-width graph was restored afterwards. Additional builds
refresh Built on as approved; final publication uses the actual current files.

MyRaidenII's existing opt-in selected-set wrapper passed its 2,000,000-instruction
startup probe and deployed-product lifecycle/Console smoke on both widths
(four checks), using the already owner-supplied archive. The wrapper removed
its temporary extracted inputs. These checks are not game-play qualification.
All 1,528 base registered cases, 130 profile cases and four optional receiver
checks passed on their first execution. No failing runtime test was retried.

The final fourteen assets have correct PE widths and refreshed November 15
2026 build dates. Product versions and filenames retain their receiving
baselines; no App source/version, INI, CMOS or external media was edited.
The [verification index](../etc/evidence/t587-s3-import-verification.json)
records all artifact SHA-256 values, command outcomes/timings and log digests.

Repository ccache counters advanced by 304 cacheable calls: 292 hits and 12
misses (96.1% hit rate), with no new cache errors. This includes test and profile
builds; it is not a performance benchmark. Generated Ninja commands retain the
repository-local cache/temp paths and 5G limit. No cache was cleared or moved.

## Delivery review and cleanup

Coordinator-role actual-diff review confirms only the approved eleven shared
files, fourteen receiving binaries and NXVM provenance/status/evidence changed.
No API/ABI, App behavior, dependency direction or package-local source divergence
was introduced. Initial standalone-gate argument mistakes are recorded above;
all corrected gates and formal CTest runs pass. Documentation and LF checks
are rerun after evidence assembly. Eight-tree source is suitable for exact
synchronization at the pinned upstream revision, not an assertion about later
SoftPC commits or every untested platform/interaction.

All owned build/test processes completed. Necessary evidence and hashes are
archived here and in the verification index; task scratch is removed using the
repository cleanup tool, retaining both stable Ninja trees and ccache.
Shared import is committed as 485f2091b; MyNES receivers as 845f1b188;
MyRaidenII receivers as 544e30426. The final NXVM P4 commit owns the ten
PC receivers and this completed qualification record. Owner acceptance
is still required; neither S3 nor T587 is silently closed.
