# M5 T539 S97 residual CPU receiver classification

## Allocation

The direct CPU-only control-transfer branch, near and far receivers move to
`test/x86/devices/cpu`, reusing the established Shared instruction fixture.

`test/app-nxvm/unit/core/devices/cpu_idt_privilege_entry_smoke.c` remains
NXVM's only direct residual receiver because it includes
`app-nxvm/devices/device_support.h`. S97 does not conceal that App dependency
by copying it into Shared or creating a new fixture/API.

## Verification

- Focused three Shared receivers plus retained IDT receiver: x64 **4/4**, x86
  **4/4**.
- T317 test-type vocabulary, CPU/PIC authority, documentation governance,
  Shared manifest, Shared corpus and `git diff --check`: passed.
- Detached repository-only full units: x64 **465/465**, x86 **465/465**.

No executable rebuild is required for this test/CMake/documentation-only
classification and ownership migration.
