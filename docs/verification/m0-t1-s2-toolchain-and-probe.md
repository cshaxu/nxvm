# M0 T1 S2 Toolchain And Probe Verification

## Commands

```powershell
cmake --preset mingw-gcc-x64
cmake --build --preset m0-probe-gcc
Get-FileHash build/mingw-gcc-x64/probes/m1-text-exit.com -Algorithm SHA256
Get-Content build/mingw-gcc-x64/probes/m1-text-exit.json
```

## Result

The generator was executed directly on the M0 authoring host with marker
`NTVDM64:M1:TEXT:OK` and exit code `42`. It produced a 31-byte COM file with
SHA-256 `c7564dde5d4bff0fb02a276d33f5134bdcc35ae32bdd2cf3c044f28cf0531112`.
The first 12 bytes matched the documented instruction sequence, the final byte
was `$`, and the JSON SHA-256 agreed.
The authoring host did not expose CMake, Ninja, MSBuild, MSVC, or GCC, so CMake
preset configuration was not executed there. The MinGW-w64 GCC smoke gate is a
historical M1 T1 S1 entry condition, not a claim that this host validated GCC.

## Supersession

M0 T5 S2 subsequently ran the GCC/CMake/Ninja smoke test successfully. Its
current evidence is [m0-t5-s2-gcc-smoke.md](m0-t5-s2-gcc-smoke.md), and M1 T1
S1 now only confirms that the recorded M0 result applies to its checkout.

M0 creates a test input only. M1 may use it as an optional baseline workload
when a lawful owner-provided local fixture can load it. M2 must run it and
verify the observed marker and exit status through the project-owned DOS
runtime.
