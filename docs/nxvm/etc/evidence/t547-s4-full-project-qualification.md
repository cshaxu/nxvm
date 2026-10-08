# M5 T547 S4 — Full Project Qualification

## Baseline

- Source baseline: `14bf118f1` (S3 closure and S4 admission).
- No production, test, asset, firmware, media or INI source was changed during
  this qualification.
- User-owned `assets/mynes/mynes_0_0_0044_{x64,x86}.exe` modifications were
  preserved and excluded from every commit.

## One-run results

| Host width | Build | Repository-only unit suite | External integration suite |
| --- | --- | --- | --- |
| x64 | `build-unit-tests`: pass | 504/506 pass; 2 fail | 22/22 pass |
| x86 | `build-unit-tests`: pass | 504/506 pass; 2 fail | 22/22 pass |

Commands were executed once per width:

```powershell
cmake --build build/mingw-gcc-<width>-release --target build-unit-tests --parallel 4
ctest --test-dir build/mingw-gcc-<width>-release --output-on-failure -L unit -j 4
cmake --build build/mingw-gcc-<width>-release --target run-integration-tests --parallel 4
```

The integration target built its own registered executable inputs before
running the external suite. No desktop test was omitted; the unit label's
three desktop-tagged tests ran as part of each full 506-case run.

## Reproducible blockers

Both widths fail exactly these two product-layer static negative tests:

1. `unit.cpu-bus-boundary-negative`: the product script still copies the
   former `test/ibmpc/board-common/core_machine_les_lds_s41_smoke.c` path.
   S3 renamed that owned source to its behavior-derived identity.
2. `unit.fdc-boundary-negative`: the product script's supposedly unmodified
   fixture assertion still requires the former `M5:T283:S2:CORE-FDC-MEDIA:OK`
   marker. S3 correctly removed task-derived current test markers.

Both are test-registration/fixture expectations outside S3's declared source
rename roots. They do not fail a shared-component runtime test or an external
integration.

## Owner-authorized reference correction

The owner authorized the narrow S4 correction on 2026-10-08. It changes only
current product test/checker references: both renamed fixture paths and the
semantic FDC success marker. A complete consumer sweep also corrected the two
same stale names in the root historical-shape and lifecycle checks, so no
current checker retains the superseded S3 identities.

`unit.cpu-bus-boundary-negative` had also repeated its static gate for every
file/dependency cross-product. Its corrected matrix has one mutation for each
of eight CPU files and one for each of the eight forbidden dependency classes;
the gate continues to read all nine CPU files, including the non-injectable
macro-only `cpu_trace.h`. Its board-test portion similarly retains all four
forbidden classes while removing repeated identical full-tree scans. This is a
test-time reduction, not an assertion or production-boundary weakening.

Focused corrective proof passed once per width:

```powershell
ctest --test-dir build/mingw-gcc-x64-release --output-on-failure -R "unit\.(cpu-bus-boundary-negative|fdc-boundary-negative)$"
ctest --test-dir build/mingw-gcc-x86-release --output-on-failure -R "unit\.(cpu-bus-boundary-negative|fdc-boundary-negative)$"
```

Both results were 2/2 passing. The renewed x64 unit run was partitioned into
six non-overlapping CTest index ranges because this host terminates a single
foreground command after roughly 30 seconds; together those ranges executed
all 506 unit cases exactly once and passed 506/506. The x86 focused result is
recorded above; its renewed complete partitioned run remains outstanding and
is not claimed by this evidence. The initial 22/22 integration result per
width remains applicable because the correction changes only static unit-test
scripts and no executable or external-test input.
