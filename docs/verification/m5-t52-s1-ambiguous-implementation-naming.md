# M5 T52 S1 Verification: Ambiguous Implementation Naming

## Change

- Renamed `src/core/machine/machine_memory.c` to
  `src/core/machine/memory_interface.c`.
- Renamed `src/core/machine/machine_port.c` to
  `src/core/machine/port_interface.c`.
- Renamed `src/core/machine/presentation.c` to
  `src/core/machine/presentation_interface.c`.
- Renamed core presentation public types, functions, and constants from
  `nxvm_core_*` / `NXVM_CORE_*` to `core_machine_*` /
  `CORE_MACHINE_*`.
- Added the task artifact target for `0.5.0052`.

## Artifact

- Path: `build/output/nxvm_0_5_0052.exe`
- SHA256:
  `5A21E18F0DCE3E92D8028E519597692659800A45614C72946A52761555004C0C`
- Banner:

```text
Neko's x86 Virtual Machine [0.5.0052]
Copyright (c) 2012-2014 Neko.
```

## Evidence

- GCC CMake/Ninja full build: passed.
- `nxvm-core-presentation-smoke`: `M5:T3:S2:PRESENTATION:OK`
- `nxvm-product-presentation-debug-smoke`:
  `M5:T6:S2:NXVM-PRESENTATION-DEBUG:OK`
- `nxvm-dos-minimal-profile-smoke`: `M3:T3:S2:DOS-MINIMAL-PROFILE:OK`
- `ntvdm64-vdm-minimal-session-smoke`: `M5:T13:S8:VDM-SESSION:OK`
- `nxvm-vm-full-authority-closure-smoke`: passed.
- `nxvm-product-console-smoke`: `M5:T6:S1:NXVM-CONSOLE:OK`
- `verify-dependency-dag`: zero known migration edges.
- `verify-live-machine-authority`: no legacy full-PC storage definitions.
- `echo EXIT | build/output/nxvm_0_5_0052.exe`: `EXITCODE=0`.

## Behavior Boundary

No Console text/grammar, debugger behavior, boot/reset ordering, media
behavior, provider lifetime, or guest-visible behavior changed.
