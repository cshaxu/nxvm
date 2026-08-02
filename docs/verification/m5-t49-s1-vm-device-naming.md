# M5 T49 S1: VM Device File Naming

T49 renamed VM-private machine device file pairs to canonical owner filenames:

- `src/vm/machine/vcmos.c/.h` to `src/vm/machine/cmos.c/.h`.
- `src/vm/machine/vdebug.c/.h` to `src/vm/machine/debug.c/.h`.
- `src/vm/machine/vfdd.c/.h` to `src/vm/machine/fdd.c/.h`.
- `src/vm/machine/vfdc.c/.h` to `src/vm/machine/fdc.c/.h`.
- `src/vm/machine/vhdd.c/.h` to `src/vm/machine/hdd.c/.h`.
- `src/vm/machine/vhdc.c/.h` to `src/vm/machine/hdc.c/.h`.

The transitional retained aliases and lifecycle names such as `vcmos`,
`vfdd`, `vfdc`, `vhdd`, `vhdc`, `vdebug`, and their `v*Init/Reset/Refresh`
functions remain direct compatibility names. No storage ownership, lifecycle
order, media behavior, controller callback, debugger behavior, Console grammar,
or boot/reset behavior changed.

## Verification

- Configured with MinGW-w64 GCC/CMake/Ninja.
- Built focused targets:
  `nxvm-vm-cmos-authority-smoke`, `nxvm-vm-fdd-authority-smoke`,
  `nxvm-vm-fdc-authority-smoke`, `nxvm-vm-hdd-authority-smoke`,
  `nxvm-vm-hdc-boundary-smoke`, `nxvm-vm-debug-authority-smoke`,
  `nxvm-vm-full-authority-closure-smoke`, `verify-dependency-dag`,
  `verify-live-machine-authority`, `nxvm-product-console-smoke`, and
  `nxvm-0-5-0049`.
- Ran:
  - `nxvm-vm-cmos-authority-smoke`: `M5:T33:S1:CMOS-AUTHORITY:OK`.
  - `nxvm-vm-fdd-authority-smoke`: `M5:T34:S1:FDD-AUTHORITY:OK`.
  - `nxvm-vm-fdc-authority-smoke`: `M5:T35:S1:FDC-AUTHORITY:OK`.
  - `nxvm-vm-hdd-authority-smoke`: `M5:T36:S1:HDD-AUTHORITY:OK`.
  - `nxvm-vm-hdc-boundary-smoke`: `M5:T37:S1:HDC-BOUNDARY:OK`.
  - `nxvm-vm-debug-authority-smoke`: `M5:T43:S1:DEBUG-AUTHORITY:OK`.
  - `nxvm-vm-full-authority-closure-smoke`: exit status 0.
  - `nxvm-product-console-smoke`: `M5:T6:S1:NXVM-CONSOLE:OK`.
- Ran the full CMake build after focused gates.
- `verify-dependency-dag`: zero known migration edges.
- `verify-live-machine-authority`: no legacy full-PC storage definitions.

## Artifact

- Path: `build/output/nxvm_0_5_0049.exe`.
- SHA-256:
  `757B8AA0FE8CD03630CE726F2585C9FBAC535475748F31568D9B95011BB380ED`.
- Banner:

```text
Neko's x86 Virtual Machine [0.5.0049]
Copyright (c) 2012-2014 Neko.
```

Piped `EXIT` reached the retained Console and returned zero.
