# M0 Toolchain Contract

M0 and later development targets 64-bit Windows 10 or Windows 11. The primary
toolchain is Visual Studio 2022 Build Tools or Visual Studio 2022 with the MSVC
x64 C/C++ workload and CMake 3.23 or later. PowerShell 5.1 or later supplies
the deterministic probe generator.

M1 enables C11 runtime compilation. The accepted compiler contract is MSVC
`/std:c11` (or a later compatible MSVC C mode), x64 target, and warnings at
`/W4 /WX` for project code.

From the repository root on a supported host:

```powershell
cmake --preset vs2022-x64
cmake --build --preset m0-probe
Get-FileHash build/vs2022-x64/probes/m1-text-exit.com -Algorithm SHA256
Get-Content build/vs2022-x64/probes/m1-text-exit.json
```

The manifest must report marker `NTVDM64:M1:TEXT:OK`, exit code `42`, the two
DOS interrupt contracts, and the SHA-256 of the adjacent COM file. M0 does not
claim the generated program can execute yet; that is M1's completion gate.
