# M3 T1 S1 Shared Core Contract Verification

## Evidence Reviewed

- Entry baseline: M1 source snapshot
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`.
- Design inputs: M2 Shared Core Architecture Requirements V2, M2 T1 S3
  parity-first header correction, and the approved M3 breakdown.
- Source surface:
  - `src/core/status.h`
  - `src/core/profile.h`
  - `src/core/machine.h`
  - `src/core/machine.c`
  - `tests/core/core_contract_smoke.c`

## Commands

```text
C:\Users\neko\AppData\Local\Microsoft\WinGet\Packages\Kitware.CMake_Microsoft.Winget.Source_8wekyb3d8bbwe\cmake-4.4.0-windows-x86_64\bin\cmake.exe --preset mingw-gcc-x64
C:\Users\neko\AppData\Local\Microsoft\WinGet\Packages\Kitware.CMake_Microsoft.Winget.Source_8wekyb3d8bbwe\cmake-4.4.0-windows-x86_64\bin\cmake.exe --build --preset m3-core-contract-gcc
build\mingw-gcc-x64\nxvm-core-contract-smoke.exe
rg -n "windows.h|win32|Win32|INT 21|ntvdm64 run|products/" src/core tests/core
git diff --check
```

## Result

- CMake configured successfully with the existing MinGW/GCC cache.
- `nxvm-core-contract-smoke` built successfully with `-Wall -Wextra
  -Wpedantic -Werror` on project-owned M3 core/test targets.
- The smoke test printed `M3:T1:S1:CORE-CONTRACT:OK`.
- Build-tree smoke executable SHA-256:
  `2c38f814c79370662f23e017a31fd3c5e53e0e893074fd18f59c12ebc3a45971`.
- The forbidden-dependency scan returned no matches.
- `git diff --check` passed.
- `build/output/` was not modified. The smoke executable is a build-tree
  verification tool, not a baseline, `nxvm.exe`, or `ntvdm64.exe` artifact.

## Contract State

The shared core now exposes a minimal linkable C11 contract through module-local
headers in `src/core/`. It defines status values, generic profile selection,
opaque Machine handles, stop reasons, run budget/result structs, and
create/reset/run/request-stop/destroy functions. It does not expose DOS,
product CLI, Win32, firmware, or imported baseline device types.
