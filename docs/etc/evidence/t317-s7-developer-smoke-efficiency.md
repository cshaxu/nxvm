# M5 T317 S7 Developer Smoke Efficiency Evidence

## Scope And Invariants

S7 changes only CMake/CTest scheduling and test classification. It retains all
194 `current-gate` test registrations and their commands, assertions, and
30-second timeouts. No product source, emulator behavior, asset, or ABI change
is part of this S.

`PROJECT_CURRENT_SMOKE_JOBS` is a cache string with default value `4`. CMake
requires the value to match the positive-integer expression `^[1-9][0-9]*$`
before it is passed to CTest. `run-current-smokes` uses that value through
`--parallel`, while `run-current-fast-smokes` uses the same scheduling but
selects `current-gate` and excludes the explicit `media` label.

## Registration Audit

The required sweep command was:

```text
rg -n "add_test\\(|LABELS|run-current|current-gate|media" CMakeLists.txt CMakePresets.json
```

It found one current-test registration route:
`project_add_current_smoke_test`. That function now adds `media` exactly when
the target belongs to `PROJECT_CURRENT_MEDIA_SMOKE_TARGETS`. The existing
four argument-bearing media loops remain the source of media-image arguments.
The only other `add_test` paths are non-current and outside S7's developer
smoke-entry scope. `run-current-smokes` remains the full target and depends on
`PROJECT_CURRENT_ALL_SMOKE_TARGETS`; the new fast target depends only on the
existing non-media `PROJECT_CURRENT_SMOKE_TARGETS` list.

## Verification

All commands used the regenerated `mingw-gcc-x64` Ninja build on 2026-08-11.

- `cmake --preset mingw-gcc-x64`: passed.
- `cmake -DPROJECT_CURRENT_SMOKE_JOBS=0 --preset mingw-gcc-x64`: failed as
  required with `PROJECT_CURRENT_SMOKE_JOBS must be a positive integer, got:
  0`; the next configuration restored `-DPROJECT_CURRENT_SMOKE_JOBS=4` and
  passed.
- `ctest --test-dir build/mingw-gcc-x64 -N -L '^current-gate$'`: listed 194
  tests, unchanged from the S7 reference baseline.
- `ctest --test-dir build/mingw-gcc-x64 -N -L '^media$'`: listed 15 tests.
- `ctest --test-dir build/mingw-gcc-x64 -N -L '^current-gate$' -LE '^media$'`:
  listed 179 tests, proving the two selections partition the 194 current
  registrations.
- `cmake --build --preset current-fast-smokes-gcc`: passed 179/179 non-media
  tests with CTest `--parallel 4`; observed real CTest time was 3.82 seconds.
- `ninja -C build/mingw-gcc-x64 run-current-smokes`: passed the complete
  194-test gate and its existing static dependencies using the default four
  jobs; the enclosing command observed 20.00 seconds elapsed. The CTest log
  records 51.83 process-seconds across `current-gate` and 37.46 process-seconds
  across the retained 15-test media subset.
- `ninja -C build/mingw-gcc-x64 verify-current-media-smoke-classification`:
  passed (`M5:T204:S1:CURRENT-MEDIA-SMOKE-CLASSIFICATION:OK`).

The first documentation-governance run exposed a packet-heading/schema issue,
which was corrected before closure; it was not a CTest or parallel-execution
failure.

## Outcome

Developers can run the fast non-media loop with:

```text
cmake --build --preset current-fast-smokes-gcc
```

The required full gate remains:

```text
cmake --build --preset current-gates-gcc
```

It now executes the unchanged full current suite with four jobs by default;
set `PROJECT_CURRENT_SMOKE_JOBS` at configure time to a different positive
integer when a host needs a different concurrency level.
