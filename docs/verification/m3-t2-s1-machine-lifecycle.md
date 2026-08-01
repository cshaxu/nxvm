# M3 T2 S1 Machine Lifecycle Verification

## Evidence Reviewed

- M2 lifecycle contract:
  `docs/history/m0-m4/m2-machine-dos-architecture.md`.
- M3 scope: `docs/planning/subtasks/m3-t2-s1.md`.
- M1 baseline snapshot:
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`.

## Commands

```text
cmake --preset mingw-gcc-x64
cmake --build --preset m3-machine-lifecycle-gcc
build/mingw-gcc-x64/nxvm-core-machine-lifecycle-smoke.exe
cmake --build --preset m3-core-contract-gcc
build/mingw-gcc-x64/nxvm-core-contract-smoke.exe
cmake --build --preset m3-machine-instance-gcc
build/mingw-gcc-x64/nxvm-core-machine-instance-smoke.exe
cmake --build --preset m1-baseline-gcc
rg -n "windows.h|win32|Win32|INT 21|ntvdm64 run|products/|nxvm-baseline" src/core tests/core
git diff --check
git diff --exit-code -- src/nxvm-baseline
```

## Result

- The lifecycle smoke printed `M3:T2:S1:LIFECYCLE:OK`.
- It verified initialized, paused, stopped, and faulted observable states;
  rejected run-before-reset, zero-budget, and run-after-stop calls; confirmed
  reset at `F000:FFF0`; and verified budget, requested-stop, and fault results.
- `machine_request_stop` uses a C11 atomic pending flag. No platform thread is
  introduced; the future execution loop observes it at an execution boundary.
- The T1 contract and instance smokes still printed their established markers.
- M1 `nxvm-baseline` built successfully without source changes. Its FDD/HDD
  watchdog evidence remains the separate temporary full-PC regression record.
- The forbidden-dependency scan returned no matches and `git diff --check`
  passed.
- Build-tree lifecycle-smoke SHA-256:
  `e548cac6c775d83ee34902ec14d2a9e55e4309cdae83ced440cd2be17ee7e972`.
- No `build/output/` artifact was created because this is a core verification
  tool, not a product executable.
