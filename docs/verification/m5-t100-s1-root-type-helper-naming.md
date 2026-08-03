# M5 T100 S1: Root Type Helper API Naming

## Result

`utilsLowerStr` and every `utilsTrace*` function are replaced by the
root-owned `ntvdm64_type_string_lower` and `ntvdm64_type_trace_*` APIs. The
existing uppercase C-runtime wrappers remain unchanged.

## Verification

- Source baseline: `7e409e9` (`M5 T99 S1`).
- Active-source old-helper scan: clean.
- `cmake --build --preset nxvm-current-gcc -j 4`: passed.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0100.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0100]`.
- Developer artifact: `build/output/nxvm_0_5_0100.exe` (not a release;
  contains no guest media), SHA-256:
  `37A06DF97A40EAC525DA05FDDB64D641DBB5D873D456E176D53C017FBC6ABDDB`.
