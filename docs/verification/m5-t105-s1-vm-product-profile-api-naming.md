# M5 T105 S1: VM Product, Profile, And Adjacent API Naming

## Result

Active VM product APIs now use `vm_product_*`; profile descriptors use
`vm_profile_*`; full-PC composition uses `vm_composition_*`; and request
transport uses `vm_platform_*`. Full-PC and default-profile reset-vector types
remain separate with explicit names, preserving their existing conversion.
No compatibility alias remains.

## Verification

- Source baseline: `64cc582` (`M5 T104 S1`).
- Active source/test scan for `consoleMain`, `nxvm_product_nxvm_*`,
  `nxvm_vm_*`, and `nxvm_full_pc_*`: clean.
- `cmake --build --preset nxvm-current-gcc -j 4`: passed.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0105.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0105]`.
- Developer artifact: `build/output/nxvm_0_5_0105.exe` (not a release;
  contains no guest media), SHA-256:
  `855F8A12D4415C3A8ABFB38B10690CA36296394470CDA6952A7ADC025B9EDF78`.
