# M5 T537 S7: Second Three-Target Audit

## Scope And Change Targets

This continuation audited `NXVM`, `MyNES`, and `Shared`.  Its only behavioral
repair is in the Shared test corpus: the Console display test wrote
`"Monitor> "` (nine cells) but asserted the next zero-based cursor column was
eight.  The assertion now expects nine; no Shared production source changed.

## Findings

| Target | Finding | Resolution |
| --- | --- | --- |
| NXVM | An old Ninja build tree registered 28 absent executables and reported three additional stale failures. | A fresh isolated build generated every registered executable. The full current-source unit and integration suites pass. The old-tree result is not a source baseline. |
| Shared | `library.console_broker_display` asserted an off-by-one cursor position. | Corrected the test expectation and its manifest SHA-256 entry. Both Apps rebuild and pass with that corpus. |
| Shared governance | The former one-task/one-scope wording could not express a task that makes independently reviewable NXVM, MyNES and Shared deliveries. | Rules now distinguish a task's declared targets from one P/commit's exactly-one target. |

No current NXVM, MyNES or Shared production finding remains unassigned.

## Verification

| Check | Result |
| --- | --- |
| Fresh NXVM x64 configuration | `REPOSITORY_BUILD_NXVM=ON`, `REPOSITORY_BUILD_MYNES=OFF` |
| Registered NXVM unit executables | 336 registered, 0 missing before CTest |
| NXVM repository-only unit | 336/336 passed, 21.97 s |
| NXVM owner-provided integration | 20/20 passed, 11.69 s |
| Fresh MyNES x64 configuration | `REPOSITORY_BUILD_MYNES=ON`, `REPOSITORY_BUILD_NXVM=OFF` |
| MyNES product suite | 53/53 passed, 24.68 s |
| Shared manifests | Lib, Common, x86 and all three matching test corpora passed |
| Shared corpus/static boundaries | Common/x86 corpus, Lib naming/types and direct dependency checks passed |
| Documentation governance | NXVM and MyNES both passed |
| Root README | Exact match with frozen `36925a4b44436fc65756c091bc7a3a2e278964cb` blob |
| Actual diff | Checked; only declared Shared governance/test and NXVM task-record changes |

The Ninja generator repeatedly stalled in CMake's compiler-ABI scratch check
on this host, including a newly allocated build directory.  The same compiler,
source settings and target selection completed through `MinGW Makefiles`; this
is recorded as a host tool-lock limitation, not a product failure or a source
workaround.  The owned stalled processes were terminated and no Ninja output is
claimed as evidence.

## Closure Review

The Shared test correction is a one-line expectation repair plus its required
manifest update.  No App source, configuration, asset or artifact is included.
The generated tracked artifacts from the verification builds were restored
before review.  All three audit views now have current evidence and no receiver
needs to be allocated.
