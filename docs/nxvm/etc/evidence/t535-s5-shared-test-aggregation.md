# T535 S5 Shared-Test Aggregation

## Owner-Approved Scope

The owner approved this shared-test corpus correction on 2026-09-22 and
explicitly authorized NXVM to make it first; SoftPC and MyNES will import the
same change later.  No product, runtime, CTest registration, asset, or public
C/C++ ABI change is included.

## One Owner Per Test List

- `test/lib/CMakeLists.txt` owns `shared-lib-tests`.
- `test/common/CMakeLists.txt` owns `shared-common-tests`.
- `test/x86/CMakeLists.txt` owns `shared-x86-tests`.
- Each aggregate depends only on the executable tests registered by that
  suite.  CMake-script checks remain CTest-only because they have no build
  executable.
- Root `CMakeLists.txt` retains only the three aggregate names in
  `PROJECT_SHARED_CORPUS_TEST_TARGETS`; it no longer repeats the suite test
  inventories.  Its remaining individual shared-target references are
  target-specific GNU/MSVC warning or stack settings, not aggregate-membership
  lists.

This preserves independent configuration and execution of Lib, Common and
x86 while deleting the second source of membership truth from the NXVM root.

## Verification

- Standalone `shared-lib-tests` build and CTest: 47/47 pass.
- Standalone `shared-common-tests` build and CTest: 18/18 pass.
- Standalone `shared-x86-tests` build and CTest: 10/10 pass.
- Root aggregate build of all three suite targets passes.
- Repository-only unit: 336/336 pass at `-j4`.
- Root-dependency sweep confirms exactly the three suite-owned aggregate
  targets are the shared inputs to `run-unit-tests`.
- Documentation governance, manifest verification, whitespace and actual-diff
  review pass.

The temporary first full-unit attempt overlapped an inherited test process and
timed out one console smoke.  The process was removed, the smoke passed alone,
and the subsequent clean complete 336/336 run is the recorded result.
