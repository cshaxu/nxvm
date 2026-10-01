# M5 T539 S94 CPU instruction receiver acceptance

## Scope

S94 moves the eight direct `x86-cpu` instruction receivers and their only
operand-probe fixture from NXVM to `test/x86/devices/cpu`:

- `cpu_setcc_smoke.c`
- `cpu_movx_smoke.c`
- `cpu_lea_smoke.c`
- `cpu_bit_test_smoke.c`
- `cpu_sign_extend_smoke.c`
- `cpu_double_shift_smoke.c`
- `cpu_bit_scan_smoke.c`
- `cpu_imul2_smoke.c`
- `support/cpu_operand_probe_fixture.h`

The Core-machine SETcc, MOVX, LEA, bit-test, sign-extend, double-shift,
bit-scan and IMUL2 receivers remain NXVM because they own the real board
route. No production source, public API, firmware, asset, INI or executable
input changed.

## Verification

- Focused Shared and retained-board receivers: x64 **16/16**, x86 **16/16**.
- `verify-t317-test-type-vocabulary`, T332, CPU/PIC authority, documentation
  governance, Shared manifest, Shared corpus and `git diff --check`: passed on
  both configured widths where applicable.
- Detached repository-only full units: x64 **458/458**, x86 **458/458**.

No executable rebuild is required because this is a test/CMake/documentation
ownership migration only.
