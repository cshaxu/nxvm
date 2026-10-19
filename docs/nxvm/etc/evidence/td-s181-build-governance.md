# Td S181 Build Governance Audit And Delivery

## Baseline And Audit

Clean17af5609a; owner-requested repository-wide standalone governance.
Actual read-only enumeration finds6056038291 logical bytes:60 directories
and3187 root files (143383157 bytes). No reparse entries or Git data found.
An elevated Win32_Process query finds no compiler, Ninja, CTest or process
using repository build files; its own read-only query is excluded. Process
use must be checked again immediately before deletion.

The old presets split identical configurations by ccache suffix; plain
presets also enable ccache unintentionally. Only CCACHE_DIR is fixed. CMake
does not constrain Ninja output. Existing standalone tooling references
retired Common roots; the historical T196 script names nonexistent targets.
The documentation checker is already correctly documentation-only and stays so.

Initial estimate: scripts/checks+300--450/-40--90, documentation separately.
No Shared source/API/manifest or artifact input change is intended.

## Concrete Existing Cleanup List

All paths below are relative to build, not arbitrary external roots.
Root-level3187 task logs/probes/archives are also reviewed candidates.
Old trees are not moved. Sensitive generated configs/ROM/fixture/snapshot
copies are preserved or identity-reviewed before deletion; none are assumed
disposable solely because they are under build. Nine downloaded research
PDFs remain useful and are preserved externally, not hidden in a cache.

| Child | Bytes | Files | Planned disposition |
| --- | ---: | ---: | --- |
| .ccache | 458922149 | 68999 | Retain valid local cache |
| audit-softpc | 19900987 | 576 | Review then remove obsolete generated output |
| audit-softpc-1451522f | 2343242 | 327 | Review then remove obsolete generated output |
| CMakeFiles | 10896619 | 491 | Review then remove obsolete generated output |
| core-special-case-audit | 134041 | 3 | Review then remove obsolete generated output |
| hotkey-audit | 102890 | 2 | Review then remove obsolete generated output |
| mingw-gcc-x64-release | 399139013 | 3034 | Review then remove obsolete generated output |
| mingw-gcc-x64-release-ccache | 937471846 | 6251 | Review then remove obsolete generated output |
| mingw-gcc-x64-release-ccache.interrupted | 528629295 | 3071 | Review then remove obsolete generated output |
| mingw-gcc-x64-release-ccache.interrupted-2 | 24859762 | 916 | Review then remove obsolete generated output |
| mingw-gcc-x64-release-ccache.interrupted-3 | 128542144 | 1147 | Review then remove obsolete generated output |
| mingw-gcc-x86-release | 617634005 | 2505 | Review then remove obsolete generated output |
| mingw-gcc-x86-release-ccache | 1298237272 | 7192 | Review then remove obsolete generated output |
| mynes-gcc-x64-release | 201010114 | 1423 | Review then remove obsolete generated output |
| mynes-gcc-x64-release-ccache | 211430486 | 1432 | Review then remove obsolete generated output |
| mynes-gcc-x86-release | 258388034 | 1455 | Review then remove obsolete generated output |
| mynes-gcc-x86-release-ccache | 17685668 | 301 | Review then remove obsolete generated output |
| myraidenii | 149277730 | 1796 | Review then remove obsolete generated output |
| pdfs-s8 | 3487319 | 5 | Review then remove obsolete generated output |
| runtime-probe | 1614654 | 8 | Review then remove obsolete generated output |
| softpc-eight-audit-20261016 | 70889817 | 1414 | Review then remove obsolete generated output |
| src | 1573568 | 77 | Review then remove obsolete generated output |
| standalone-emulator-x64 | 712655 | 27 | Review then remove obsolete generated output |
| standalone-emulator-x64-source | 581715 | 19 | Review then remove obsolete generated output |
| standalone-emulator-x86 | 852636 | 27 | Review then remove obsolete generated output |
| standalone-emulator-x86-source | 753579 | 19 | Review then remove obsolete generated output |
| standalone-lib-x64 | 727964 | 17 | Review then remove obsolete generated output |
| standalone-lib-x64-source | 676978 | 17 | Review then remove obsolete generated output |
| standalone-lib-x86 | 907332 | 17 | Review then remove obsolete generated output |
| standalone-lib-x86-source | 846899 | 17 | Review then remove obsolete generated output |
| standalone-x86-core-x64 | 93960181 | 378 | Review then remove obsolete generated output |
| standalone-x86-core-x64-source | 623343 | 15 | Review then remove obsolete generated output |
| standalone-x86-core-x86 | 105800932 | 378 | Review then remove obsolete generated output |
| standalone-x86-core-x86-source | 762839 | 15 | Review then remove obsolete generated output |
| standalone-x86-debug-x64 | 682599 | 29 | Review then remove obsolete generated output |
| standalone-x86-debug-x64-source | 630175 | 20 | Review then remove obsolete generated output |
| standalone-x86-debug-x86 | 822339 | 29 | Review then remove obsolete generated output |
| standalone-x86-debug-x86-source | 769692 | 20 | Review then remove obsolete generated output |
| standalone-x86-ibmpc-x64 | 695310 | 31 | Review then remove obsolete generated output |
| standalone-x86-ibmpc-x64-source | 651275 | 21 | Review then remove obsolete generated output |
| standalone-x86-ibmpc-x86 | 834991 | 31 | Review then remove obsolete generated output |
| standalone-x86-ibmpc-x86-source | 790823 | 21 | Review then remove obsolete generated output |
| standalone-x86-nxvm-x64 | 143626973 | 1448 | Review then remove obsolete generated output |
| standalone-x86-nxvm-x64-source | 857310 | 23 | Review then remove obsolete generated output |
| standalone-x86-nxvm-x86 | 163276905 | 1448 | Review then remove obsolete generated output |
| standalone-x86-nxvm-x86-source | 997109 | 23 | Review then remove obsolete generated output |
| t561-boundary-probes | 0 | 0 | Review then remove obsolete generated output |
| t561-s7-negative | 77 | 3 | Review then remove obsolete generated output |
| t561-s8-boundary-selftest | 0 | 0 | Review then remove obsolete generated output |
| t561-s8-negative-probe | 5381170 | 772 | Review then remove obsolete generated output |
| t561-s8-route-probe | 236 | 1 | Review then remove obsolete generated output |
| t561-s9-boundary | 0 | 0 | Review then remove obsolete generated output |
| t561-upstream | 2046465 | 264 | Review then remove obsolete generated output |
| t562-s1-upstream | 2100652 | 265 | Review then remove obsolete generated output |
| t563-s9-nxvm-baseline | 13240610 | 1236 | Review then remove obsolete generated output |
| t563-s9-softpc-4a675c003 | 2221333 | 303 | Review then remove obsolete generated output |
| t564-s1 | 7744824 | 344 | Review then remove obsolete generated output |
| td-s180-softpc-head | 2107886 | 268 | Review then remove obsolete generated output |
| test | 12323624 | 98 | Review then remove obsolete generated output |
| window-aspect-audit | 1475048 | 2 | Review then remove obsolete generated output |

Existing committed S46 results, artifact and media summaries preserve the
latest qualification; raw logs/probes are not permanent evidence authorities.
Source policy prohibits committing protected bytes or machine-local paths;
only relative inventories, actual hashes and neutral results are retained.

## Pre-Apply Disposition

The actual full preview preflights3247 exact targets /44261 files /
5597116191 bytes and deletes zero. It retains .ccache, .ninja and this task's
temporary td-s181 directory until results are archived. No Git checkout exists;
all10 root source/baseline archives are readable and contain no .git metadata.
The additional nested source archive is covered by the sensitive-file review.

[Sensitive identities](td-s181-sensitive-inputs.tsv) records310 actual hashes:
known compiler probes, code-owned test/firmware/snapshot outputs, CMake YAML
metadata and source archives are reviewed generated inputs. The failed safety
test's synthetic fixture is owned test data, not a real disk. There are no
remaining unclassified entries. Thirteen INI copies, nine research PDFs and
one uncertain diagnostic ROM are copied outside the repository and verified
byte-for-byte before cleanup:23 files /96126758 bytes. Their original relative
paths/hashes identify them without committing their bytes or machine paths.
No asset INI/media/ROM or user global cache is touched. The external copy is
preservation of these inputs, not a relocation of obsolete trees or task logs.

## Verified Result And Bounded Failures

Both real GCC Release root graphs freshly configure below .ninja; each builds
the complete repository unit-binary target in1494 Ninja steps, with no product
deployment target. The following unchanged builds return `ninja: no work to do.`
Plain then ccache presets reconfigure the same directories; plain explicitly
clears the launcher, cached commands contain all three fixed environment
settings. No old CMake tree is moved. Four actual Lib/Emulator source/test
standalone configurations also accept the injected single root policy.

Each width passes both registered repository governance tests. The real
compiler fixture proves disabled cache, enabled cache, direct build under
hostile caller cache/temp/size variables, no-change object timestamp, one new
local cache hit after an owned rebuild, and enabled-to-disabled clearing.
The wrong caller cache/temp paths are not created. Final outputs/XML hashes
and phase-log hashes are archived in [results](td-s181-results.json), not left
inside caches. CTest-native metadata remains normal build metadata.

The cleanup test calls the real tool within one PowerShell process. It covers
preview, Apply acknowledgement, absolute/traversal/dot/short-name aliases,
cache roots/children, overlapping targets, incomplete final coverage, unreviewed
or wrong-hash media, Git and junction/ancestor traversal. It proves all-candidate
preflight before any deletion and actual deletion of exactly reviewed files.
The temporary hash-set lookup avoids quadratic pair comparisons across3187
root files; it is local preflight data, not a cache manager or persistent state.

Initial tests exposed a Windows PowerShell module auto-loading scope issue,
incorrect ccache statistic field names and a caller-env leak in the test's
own statistics query. Those test/tool defects are corrected explicitly. A
concurrent build's initial x86 registration preceded the new CompilerId
parameter; real reconfiguration resolves it. The restricted sandbox refuses
test junction creation, so final safety tests run with that operation allowed;
the assertion is not skipped or reclassified. Final dual-width tests pass.

Apply repeats actual process and full safety checks, then deletes all3247
previewed targets /44261 files /5597116191 logical bytes. The measured volume
free-space increase is5662666752 bytes; allocation and other volume activity
make logical byte accounting the portable report. It leaves only both cache
roots and this task's bounded temporary output, removed after archiving results.
No Git checkout, protected original or unknown unpreserved file is deleted.

Three product documentation gates and whitespace checks pass. The global
Types gate includes the newly staged tooling fixture and passes1398 tracked
code/script inputs. All ten deployed artifact SHA256/size identities still
match S46. No shared source/test corpus/manifest changes, product behavior or
compiler optimization changes occur. Historical evidence is not globally
rewritten. The stale T196 script has no current targets/callers and is removed;
MyNES standalone tooling replaces retired Common paths at its real owner.

Full emulator runtime unit/integration suites, native devices, desktop/RDP,
Clang/MSVC/POSIX runtime and the complete standalone native suites are not
rerun or claimed by this tooling-only Td. Shared corpus entry points stay
portable; repository standalone tooling injects the repository policy rather
than making a shared corpus depend on outer build infrastructure. Raw external
consumer standalone builds are not governed by a second copied policy.

## Final Cleanup And Review

After archiving actual results and hashes, the same real tool previews and
removes td-s181 with RequireClean:23 files /346156 bytes. A fresh actual
process query finds no build/test work. Total reviewed removal is5597462347
logical bytes,44284 files; build now contains exactly .ccache and .ninja.
The reusable cache directories are never moved or cleared. The23 preserved
inputs remain externally hash-identical and are recoverable; obsolete generated
outputs are reproducible, not retained as hidden third caches.

Actual-change coordinator review confirms one CMake route, one cleanup owner,
same-process safety tests, no copied preset parser or documentation cleanup
gate, no background manager and no shared-corpus or runtime behavior changes.
Added local helpers have concrete query/refusal responsibilities, not forwarding
adapters. Actual root cache settings and launcher commands agree with presets.
Windows aliases, cache roots, junctions and sensitive identities are checked
before removal; the caller still owns exclusion of concurrent writers.

Code/build/scripts/checks:383 added /47 deleted /net336, including all new
tracked tooling fixtures. Documentation and evidence are counted separately
in the final delivery record: documentation+162/-52/net110; evidence+664/-0.
Counts use git diff HEAD --numstat plus actual new-file lines, excluding build
outputs. Positive code size is the bounded CMake contract,
actual safety owner and direct regression proof, not a cache management layer.
No production binary update is needed: all ten deployed hashes/sizes match
the accepted pre-task baseline. The complete Td delivery awaits owner review;
NXVM T564 and MyRaidenII T2 are not closed or advanced.

Pushed delivery: Shared P1 `25e828def`, MyNES P2 `51a28e226` and MyRaidenII
P3 `8f0245cff`; the NXVM P4 receipt carries this complete audit and actual
cleanup/result records. Each P has one target and is pushed immediately.
