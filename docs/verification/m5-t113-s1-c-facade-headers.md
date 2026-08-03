# M5 T113 S1: ISO C Header Ownership

## Result

`src/type.h` owns all ISO C includes required by active project code and
tests, including `stdint.h`. The old hand-written Win32 fixed-width typedef
branch is removed in favor of the standard header; `memory.h` is removed.
Platform SDK headers remain local to their adapters. The
`verify-c-facade-headers` CMake target rejects future direct ISO C includes
outside `type.h`.

## Verification

- `Verify-CFacadeHeaders.ps1`: emitted `M5:T113:C-FACADE-HEADERS:OK`.
- `cmake --preset mingw-gcc-x64` and
  `cmake --build --preset nxvm-current-gates-gcc`: passed, including the new
  `verify-c-facade-headers` target.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0113.exe` accepted `help`, `info`, and `exit`; retained Console
  output and idle device state were observed.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0113]`.
- Developer artifact: `build/output/nxvm_0_5_0113.exe` (not a release;
  contains no guest media), SHA-256:
  `33B07A66735EE423E9CD8882C77AB8497AF429CA19C714605A2FFC02E4420FAA`.
