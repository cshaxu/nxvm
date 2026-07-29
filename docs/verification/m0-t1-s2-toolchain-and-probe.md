# M0 T1 S2 Toolchain And Probe Verification

## Commands

```powershell
cmake --preset vs2022-x64
cmake --build --preset m0-probe
Get-FileHash build/vs2022-x64/probes/m1-text-exit.com -Algorithm SHA256
Get-Content build/vs2022-x64/probes/m1-text-exit.json
```

## Result

The generator was executed directly on the M0 authoring host with marker
`NTVDM64:M1:TEXT:OK` and exit code `42`. It produced a 31-byte COM file with
SHA-256 `c7564dde5d4bff0fb02a276d33f5134bdcc35ae32bdd2cf3c044f28cf0531112`.
The first 12 bytes matched the documented instruction sequence, the final byte
was `$`, and the JSON SHA-256 agreed.
The authoring host did not expose CMake, Ninja, MSBuild, or MSVC, so CMake preset
configuration was not executed there. This remains a required first-run check
on the supported Visual Studio 2022 toolchain before M1 starts.

M0 creates a test input only. M1 must run it and verify observed marker and
exit status through the project-owned DOS runtime.
