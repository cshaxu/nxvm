# M5 T103 S1: VM Composition And Machine API Naming

## Result

The active VM composition lifecycle and block-binding APIs now use
`vm_composition_*`; HDC lifecycle APIs use `vm_machine_hdc_*`; and CPU-probe
types/functions use `vm_composition_cpu_probe_*`. No old API or compatibility
alias remains in active source or tests.

## Verification

- Source baseline: `5a38e5e` (`M5 T102 S1`).
- Active source/test scan for the replaced APIs: clean.
- `cmake --build --preset nxvm-current-gcc -j 4`: passed.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0103.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0103]`.
- Developer artifact: `build/output/nxvm_0_5_0103.exe` (not a release;
  contains no guest media), SHA-256:
  `5266EF720BAB26FFA6F9127609047177EA9F5BD9052E00BCE0FA2EB1CB5AB36C`.
