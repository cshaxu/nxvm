# M5 T102 S1: Core Product API Naming

## Result

Shared product APIs now use `core_product_debug_main`,
`core_product_utils_*`, `core_product_execution_context_*`, and
`core_product_runtime_*`. The lexical names `debug`, `aasm`, `dasm`, and
`xasm32` remain intact; no compatibility alias remains.

## Verification

- Source baseline: `7cd49f6` (`M5 T101 S1`).
- Active source/test old-API scan: clean.
- `cmake --build --preset nxvm-current-gcc -j 4`: passed after one missed
  `composition_loop.c` execution-context prefix was corrected.
- `cmake --build --preset nxvm-current-gates-gcc -j 4`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0102.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; `info` reported the FDD as `inserted`.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0102]`.
- Developer artifact: `build/output/nxvm_0_5_0102.exe` (not a release;
  contains no guest media), SHA-256:
  `F17B81E277E725A833460BBC36637675CD47582FABA3B9C7E6C57D9D867C1594`.
