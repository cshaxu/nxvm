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
integration. S4 remains active until a separately scoped corrective change
updates these product-level negative checks and a new complete unit run proves
the result; this one-run evidence is not retried.
