# M0 T5 S2 GCC Toolchain Smoke Verification

## Environment

- Host target: x64 Windows 7 through Windows 11 policy; validation executed on
  the current x64 Windows host.
- Compiler: WinLibs MinGW-w64 GCC 16.1.0, x86_64 MSVCRT POSIX/SEH.
- Build tool: Ninja 1.13.2.
- Configuration tool: CMake 4.4.0.

## Commands

```powershell
cmake --preset mingw-gcc-x64
cmake --build --preset m0-probe-gcc
Get-FileHash build/mingw-gcc-x64/probes/m1-text-exit.com -Algorithm SHA256
Get-Content build/mingw-gcc-x64/probes/m1-text-exit.json
```

## Result

CMake identified GNU C 16.1.0, validated C11 compiler features and a 64-bit
target, and generated the Ninja build tree. `generate-m1-probe` succeeded.
The generated COM file SHA-256 was
`c7564dde5d4bff0fb02a276d33f5134bdcc35ae32bdd2cf3c044f28cf0531112`; the
manifest reported marker `NTVDM64:M1:TEXT:OK` and exit code `42`.

This verifies the M0 toolchain and probe laboratory only. It does not claim
that a DOS runtime exists or that the generated program executes under ntvdm64.
