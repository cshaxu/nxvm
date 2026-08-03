# M5 T99 S1: Root Type Include Boundary

## Result

The 17 active `core/machine/vglobal.h` callers now include `type.h` directly.
`src/core/machine/vglobal.h` is deleted; no compatibility forwarding include
remains.

## Verification

- Source baseline: `d95b629` (`M5 T108 S1`).
- Active-source `vglobal.h` include scan: clean.
- `cmake --build --preset nxvm-current-gcc -j 4`: passed.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0099.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0099]`.
- Developer artifact: `build/output/nxvm_0_5_0099.exe` (not a release;
  contains no guest media), SHA-256:
  `37D94FE76C2CE7D66A07D681CDACED49B285E9A292BFE214FE15F638708C0E10`.
