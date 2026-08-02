# M5 T48 S2: Core Device File Naming

T48 S2 renamed the shared core device file pairs and their current-object
functions without changing device behavior:

- `src/core/machine/keyboard_controller.c/.h` to
  `src/core/machine/kbc.c/.h`.
- `src/core/machine/video_adapter.c/.h` to `src/core/machine/vadp.c/.h`.
- `core_machine_keyboard_controller_*` to `core_machine_kbc_*`.
- `core_machine_video_adapter_*` to `core_machine_vadp_*`.

The transitional retained aliases `vkbc` and `vvadp` remain direct accessors
for the current live machine object. No KBC or VADP state layout, lifecycle
callback, port registration, display snapshot behavior, Console grammar, or
debugger behavior changed.

## Verification

- Configured with MinGW-w64 GCC/CMake/Ninja.
- Built focused targets:
  `nxvm-vm-kbc-authority-smoke`, `nxvm-vm-vadp-authority-smoke`,
  `nxvm-vm-full-authority-closure-smoke`, `verify-dependency-dag`,
  `verify-live-machine-authority`, `nxvm-product-console-smoke`,
  `ntvdm64-version-smoke`, and `nxvm-0-5-0048`.
- Ran:
  - `nxvm-vm-kbc-authority-smoke`:
    `M5:T30:S1:KBC-AUTHORITY:OK`.
  - `nxvm-vm-vadp-authority-smoke`:
    `M5:T31:S1:VADP-AUTHORITY:OK`.
  - `nxvm-vm-full-authority-closure-smoke`: exit status 0.
  - `nxvm-product-console-smoke`: `M5:T6:S1:NXVM-CONSOLE:OK`.
  - `ntvdm64-version-smoke`: `M5:T47:S1:VERSION-FOUNDATION:OK`.
- Ran the full CMake build after the focused gates.
- `verify-dependency-dag`: zero known migration edges.
- `verify-live-machine-authority`: no legacy full-PC storage definitions.

## Artifact

- Path: `build/output/nxvm_0_5_0048.exe`.
- SHA-256:
  `BFD634E26371FE20064C0DB7A9FBDAC6C0F5F73C202986B0F1423EDF15C53DD2`.
- Banner:

```text
Neko's x86 Virtual Machine [0.5.0048]
Copyright (c) 2012-2014 Neko.
```

Piped `EXIT` reached the retained Console and returned zero.
