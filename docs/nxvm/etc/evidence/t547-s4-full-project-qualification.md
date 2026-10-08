# M5 T547 S4 — Full Project Qualification

## Baseline

- Admission baseline: `14bf118f1` (S3 closure and S4 admission).
- Final qualification baseline: `9ab7ed6b9`.  S4 contains the owner-approved
  product-test reference correction `16e008415`; it changes no production
  source, firmware, media or INI.  `9ab7ed6b9` then publishes the matching
  rebuilt MyNES 0044 pair.

## One-run results

| Host width | Build | Repository-only unit suite | External integration suite |
| --- | --- | --- | --- |
| x64 | `build-unit-tests`: pass | 506/506 pass | 22/22 pass |
| x86 | `build-unit-tests`: pass | 506/506 pass | 22/22 pass |

Commands were executed once per width:

```powershell
cmake --build build/mingw-gcc-<width>-release --target build-unit-tests --parallel 4
ctest --test-dir build/mingw-gcc-<width>-release --output-on-failure -L unit -j 4
cmake --build build/mingw-gcc-<width>-release --target run-integration-tests --parallel 4
```

The integration target built its own registered executable inputs before
running the external suite. No desktop test was omitted; the unit label's
three desktop-tagged tests ran as part of each full 506-case run.

The unit suite includes all four shared test packages: the currently
registered label populations are Lib 36, Common 14, x86 176 and IBM PC 176 per
host width.  The remaining cases are NXVM product and cross-package tests.
No shared package was excluded.

The MyNES product target was also built and executed after the corrected
baseline: 57/57 passed on x64 and 57/57 passed on x86.  This includes its
product unit, integration and native desktop smoke cases.

## Corrected blockers

Both widths fail exactly these two product-layer static negative tests:

1. `unit.cpu-bus-boundary-negative`: the product script still copies the
   former `test/ibmpc/board-common/core_machine_les_lds_s41_smoke.c` path.
   S3 renamed that owned source to its behavior-derived identity.
2. `unit.fdc-boundary-negative`: the product script's supposedly unmodified
   fixture assertion still requires the former `M5:T283:S2:CORE-FDC-MEDIA:OK`
   marker. S3 correctly removed task-derived current test markers.

Both were test-registration/fixture expectations outside S3's declared source
rename roots. They did not indicate a shared-component runtime or external
integration defect.

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

Both results were 2/2 passing.  The renewed x64 and x86 unit runs were each
partitioned into non-overlapping CTest index ranges because this host terminates
a single foreground command after roughly 30 seconds; together the ranges
executed all 506 unit cases exactly once per width and passed 506/506.

The external integration targets were then re-run, rather than merely carried
forward from the initial baseline: 22/22 passed on x64 and 22/22 passed on x86.
Those runs include the available ROM/media-backed DOS, video, FDC/HDC, Windows
3.1 checkpoint and floppy boot-matrix cases.  No desktop case was omitted:
desktop-labelled checks run through the unit and MyNES product targets above.
