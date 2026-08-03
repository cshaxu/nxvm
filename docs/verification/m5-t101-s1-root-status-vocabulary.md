# M5 T101 S1: Root Status Vocabulary

## Result

The root status type is `ntvdm64_status`; its constants are
`NTVDM64_STATUS_OK`, `NTVDM64_STATUS_INVALID_ARGUMENT`,
`NTVDM64_STATUS_INVALID_STATE`, `NTVDM64_STATUS_UNSUPPORTED`,
`NTVDM64_STATUS_NO_MEMORY`, and `NTVDM64_STATUS_FAULT`. No old status alias
remains in active source or tests.

## T107 Inventory

`type.h` still contains retained, not-yet-renamed `t_string`, `t_strptr`,
`t_nubit*`, `t_nsbit*`, `t_float*`, `t_bool`, `t_vaddrcc`, `t_faddrcc`, their
`p_*`/`d_*` pointer-access macros, bit/width helpers, boolean/zero/max/MSB
macros, BCD helpers, execution helper, and trace-control macros. T107 owns
their replacement/removal decision; this task changes none of them.

## Verification

- Source baseline: `cf3e741` (`M5 T100 S1`).
- Active source/test old-status scan: clean.
- `cmake --build --preset nxvm-current-gcc -j 4`: passed.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0101.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0101]`.
- Developer artifact: `build/output/nxvm_0_5_0101.exe` (not a release;
  contains no guest media), SHA-256:
  `42972A5001B7F96E4F2F3E3C2E56EC0CCC1447CF5B2FD115D59ED621A54E5C7D`.
