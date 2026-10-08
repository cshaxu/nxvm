# T547 S2 Shared Four-Package Test Stability

## Scope

S2 continues the pushed S1 reconciliation at `26c013bba`.  It validates the
four independently registered shared test packages (`lib`, `common`, `x86`,
and `ibmpc`) once on each supported host width.  No shared production source,
public ABI, App source, asset, firmware, media, INI, or MyNES file is changed.

## Root Cause And Repair

The package tests themselves did not need retries or longer per-test budgets.
On this host, running the CPU-heavy `x86`/`ibmpc` smoke population at the old
aggregate default of eight concurrent processes can starve individual tests
beyond their existing 30-second CTest budget.  The same named binaries pass in
under one second when not contending, and all four package aggregates pass at
four jobs.

`cmake/nxvm/NxvmProduct.cmake` therefore changes only the default
`PROJECT_UNIT_TEST_JOBS` from 8 to 4.  This is the highest concurrency verified
here for the complete CPU-heavy package mix.  It does not change a test's
timeout, assertion, retry behavior, fixture contract, resource lock, or
production behavior.  Existing build trees were explicitly reconfigured with
`-DPROJECT_UNIT_TEST_JOBS=4`, so the aggregate setting used for the proof does
not rely on an old cache entry.

The long `library.types-layout-selftest` is a CMake static check, not an
intermittent native test.  Its 47-second x64 and 124-second x86 completions are
valid registered results; the earlier apparent failure was an external
30-second sampling cap, not CTest failure.

## Single-Run Qualification

Each result below is one complete aggregate run after its configured build.

| Package | x64 | x86 |
| --- | ---: | ---: |
| `test/lib` | 51/51 passed (46.23 s) | 51/51 passed (135.75 s) |
| `test/common` | 20/20 passed (49.68 s) | 20/20 passed (35.29 s) |
| `test/x86` | 182/182 passed (45.25 s) | 182/182 passed (95.06 s) |
| `test/ibmpc` | 182/182 passed (126.46 s) | 182/182 passed (118.38 s) |

The Lib native Window/Console tests retain their precise `native_desktop`
resource lock.  Other tests remain parallel; no package-wide serialization was
introduced.

The complete repository-only unit aggregate also passes once per width at the
same four-job setting: x64 506/506 in 118.43 seconds and x86 506/506 in 111.04
seconds.  An accidental unlabelled `ctest` invocation was stopped immediately
when it began external integration cases; it is not included in either unit or
integration evidence.

## Similar-Path Sweep

- Lib's native desktop tests: covered by their existing precise resource lock;
  both widths pass in the complete Lib aggregate.
- Long static self-tests: allowed to complete in CTest rather than being judged
  by an external controller timeout.
- x86 and IBM PC CPU-heavy tests: no individual timeout or assertion change;
  aggregate contention is constrained by the one shared default.
- Per-test work directories, native resource ownership, and the x86 negative
  fixture isolation imported in S1 remain in force.

## Remaining Boundary

This proof qualifies the four shared packages only.  It does not claim App
unit/integration qualification, external assets, or user-owned MyNES artifacts.
