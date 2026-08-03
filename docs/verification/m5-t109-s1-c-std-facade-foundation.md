# M5 T109 S1: C And ISO C Facade Foundation

## Result

`type.*` now defines `C_*` scalar aliases and `STD_*` ISO C type/object
vocabulary. Existing root wrappers and active callers use `STD_*`; new wrapper
surfaces include allocation, file positioning/character I/O, bounded
formatting, time, and ctype support. No scalar keyword or Win32 call-site
migration occurs in this task.

## Verification

- Source baseline: `236b7d4` (`M5 T108 S2`).
- Active source/test legacy root-wrapper scan: clean.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0109.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0109]`.
- Developer artifact: `build/output/nxvm_0_5_0109.exe` (not a release;
  contains no guest media), SHA-256:
  `CEB373E234FC89876595710EF663D8AF1BC877F79B86C48E13CF2EA9A7106A4D`.
