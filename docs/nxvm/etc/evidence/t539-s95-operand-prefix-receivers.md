# M5 T539 S95 operand and prefix receiver acceptance

## Scope

S95 moves `cpu_operand_address_smoke.c` and
`cpu_prefix_attributes_s64_smoke.c` into `test/x86/devices/cpu`, where both
reuse the existing Shared instruction fixture. The direct `x86-cpu` tests own
instruction semantics. The Core-machine operand/address and prefix-attribute
receivers remain NXVM because they exercise board routes.

No production source, public API, firmware, asset, INI or executable input
changed.

## Verification

- Focused Shared and retained-board receivers: x64 **3/3**, x86 **3/3**.
- T317 test-type vocabulary, CPU/PIC authority, documentation governance,
  Shared manifest, Shared corpus and `git diff --check`: passed.
- Detached repository-only full units: x64 **460/460**, x86 **460/460**.

No executable rebuild is required for this test/CMake/documentation-only
ownership migration.
