# M3 T1 S2 Machine Instance Verification

## Evidence Reviewed

- Entry baseline: M1 source snapshot
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`.
- Approved scope: `docs/planning/subtasks/m3-t1-s2.md`.
- Behavioral reference and ownership record:
  `docs/provenance/m3-t1-s2-machine-instance-shell.md`.

## Commands

```text
cmake --preset mingw-gcc-x64
cmake --build --preset m3-machine-instance-gcc
build/mingw-gcc-x64/nxvm-core-machine-instance-smoke.exe
cmake --build --preset m3-core-contract-gcc
build/mingw-gcc-x64/nxvm-core-contract-smoke.exe
cmake --build --preset m1-baseline-gcc
rg -n "windows.h|win32|Win32|INT 21|ntvdm64 run|products/|nxvm-baseline" src/core tests/core
git diff --check
git diff --exit-code -- src/nxvm-baseline
```

## Result

- GCC built `nxvm-core` with `-Wall -Wextra -Wpedantic -Werror`.
- The instance smoke printed `M3:T1:S2:MACHINE-INSTANCE:OK`.
- It created two 2 MiB Machines, verified their CPU reset state, independent
  RAM and port state, A20-on/off behavior, checked reset clearing, and typed
  port dispatch.
- The existing contract smoke still printed `M3:T1:S1:CORE-CONTRACT:OK`.
- The M1 `nxvm-baseline` target built successfully with its known imported
  warnings. `git diff --exit-code -- src/nxvm-baseline` confirmed no baseline
  source change, so the recorded FDD/HDD ten-second watchdog evidence remains
  applicable to that temporary regression adapter.
- The forbidden-dependency scan returned no matches and `git diff --check`
  passed.
- Build-tree instance-smoke SHA-256:
  `406737aaa5430ea4d03294ec20db5a5f7260a7a317009261a4207ca79264853f`.
- No `build/output/` artifact was created: this is a core verification tool,
  not yet an `nxvm.exe` or `ntvdm64.exe` developer artifact.

## Boundary State

New core code owns its CPU/RAM/port instances. The M1 baseline remains a
separate global-state executable and is not represented as a reusable core
Machine until the explicit `nxvm.full_pc` composition work in M3 T3.
