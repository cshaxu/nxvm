# T500 S8: Repository-Only Unit Matrix Convergence

## Result

S8 removes duplicate or invalid test-only routes and registers the retained
repository-only owner contracts through the sole CMake unit list. The final
test/CMake delta is negative: empty alias checks, a test-helper self-test, an
unbounded session loop, and duplicated Model-339 controller/pacing scripts are
removed. The retained coverage adds only existing owner-local contracts.

## Matrix ownership

- CPU execution remains owned by Core instruction fixtures and ledger runners.
- Core lifecycle, trace, external-time and Model-339 profile contracts each
  retain one owner-local route.
- VM session requests use accepted and rejected request matrices. Model 339
  now rejects unsupported CPU/FPU/memory/floppy overrides instead of accepting
  and silently discarding them.
- Controller deadline, KBC, PIT/RTC, standard/turbo and external-media paths
  remain with their existing Core, VM policy, or integration owners.

## Verification

`ctest --test-dir build/mingw-gcc-x64 --output-on-failure -L unit -j 4`
completed with 312/312 passing repository-only unit tests.
