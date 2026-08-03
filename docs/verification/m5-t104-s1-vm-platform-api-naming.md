# M5 T104 S1: VM Platform API Naming

## Result

Active VM platform hub, Linux, Win32, `win32app`, and `win32con` APIs now use
`vm_platform_*` ownership prefixes. The approved backend tags remain lexical
components of those names; no compatibility alias remains.

## Verification

- Source baseline: `02d9a43` (`M5 T103 S1`).
- Active source/test scan for replaced unscoped platform APIs: clean.
- `cmake --build --preset nxvm-current-gcc -j 4`: passed.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0104.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0104]`.
- Developer artifact: `build/output/nxvm_0_5_0104.exe` (not a release;
  contains no guest media), SHA-256:
  `F618C650D26617C043C343EEC1FC7BE34A764DD755DC333D9AD783E628146042`.
