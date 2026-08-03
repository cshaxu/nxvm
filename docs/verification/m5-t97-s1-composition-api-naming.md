# M5 T97 S1: VM Composition API Naming

## Result

The active full-PC composition lifecycle surface is now explicit:

- `vm_composition_providers_initialize`
- `vm_composition_providers_refresh`
- `vm_composition_providers_reset`
- `vm_composition_providers_finalize`
- `vm_composition_print_machine`

Initialization, refresh, reset, and finalization call order is unchanged.
The static source scan found no active use of the replaced symbols.

## Deliberate Retentions

`kbc`, `vadp`, `win32app`, `win32con`, `linuxapp`, `linuxcon`, `w32a`, `w32p`,
`xasm32`, `aasm`, `dasm`, and `debug` are approved compact names. `qdx`,
`qdcga`, and `qdkeyb` remain named as documented default-profile QDX hacks.
Historical documents retain the original `vmachine` terminology as provenance.

## Verification

- Source baseline: `2111ad4` (`M5 T96 S1`).
- `cmake --build --preset nxvm-current-gcc -j 4`: passed.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed, including
  Console lifecycle, executor closure, facade ownership, dependency DAG, and
  session-readiness gates.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0097.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0097]`.
- Developer artifact: `build/output/nxvm_0_5_0097.exe` (not a release;
  contains no guest media), SHA-256:
  `6BF168A227BFA354EFCD7B4572EC87907A1A5C758DA6EF0933F080CDA83E6FF9`.
