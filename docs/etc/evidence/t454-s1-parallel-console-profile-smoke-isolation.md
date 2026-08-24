# T454 S1: Parallel Console Profile-Smoke Isolation

## Mechanism Inventory

The Console catalog enumerates every YAML file in the supplied directory. Three
current-gate tests create a fixed YAML fixture and invoke that catalog with
`.`: `nxvm_console_lifecycle_smoke.c`,
`nxvm_console_memory_roundtrip_smoke.c`, and
`vm_model40_console_s20_smoke.c`. Their distinct file names did not prevent
one test's fixture from changing another test's catalog and menu ordinal when
CTest used its shared build directory.

`CMakeLists.txt` now assigns exactly those three registered tests one owned
directory below `build/mingw-gcc-x64/tests/<target>`. The existing test cleanup
removes each fixture; product catalog discovery, Console input restoration, and
all production profile paths remain unchanged. No source-working-directory
fixture exists after the runs.

## Verification

- Targeted configure and build: all three targets were already current after
  regeneration.
- Serial control (`--parallel 1`): 3/3 passed.
- Parallel replay (`--parallel 3 --repeat until-fail:3`): each target ran
  three times; 3/3 passed with no retry/failure.
- The generated CTest registration records three distinct
  `WORKING_DIRECTORY` values. The five possible source-root YAML/ROM fixture
  paths were absent after verification.
- `current-gates-gcc` built the configured targets and ran the 292-test
  current-gate set. The three isolated tests passed. The independent
  `current.core-machine-8086-decoder-ledger-s5` verifier failed its joint
  source-form predicate; direct replay reproduces it. This CPU evidence
  mismatch is recorded in `TODO.md` and is not a Console isolation result.

## Change Accounting

Tracked code/test paths: `CMakeLists.txt` only, +14/-0 lines. The explicit
three-item list is the complete discovered mechanism set; it introduces no
new helper, target, catalog, production path, or test-only public interface.
