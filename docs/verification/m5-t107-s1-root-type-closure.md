# M5 T107 S1: Root Type Vocabulary And Closure

## Result

Root numeric, string, Boolean, address, and trace typedefs now use
`ntvdm64_type_*`. Root bit, mask, BCD, pointer/dereference, execution, and
trace-control macros now use `NTVDM64_TYPE_*`. The approved uppercase
C-runtime wrappers remain unchanged. Module-owned legacy structures such as
`t_cpu` and `t_pic` are not root aliases and are outside this task.

## Verification

- Source baseline: `cb3936f` (`M5 T106 S1`).
- Active scan for former root typedefs/macros: clean.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0107.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0107]`.
- Developer artifact: `build/output/nxvm_0_5_0107.exe` (not a release;
  contains no guest media), SHA-256:
  `6F7200846EAAE9B0FB234D307BAA07F8C72AA9BF6BBF3C1B3D81D8A3F851A0E5`.
