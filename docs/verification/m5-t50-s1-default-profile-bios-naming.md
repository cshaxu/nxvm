# M5 T50 S1: Default Profile BIOS File Naming

T50 renamed the default-profile BIOS firmware file pair:

- `src/vm/profile/default_profile/firmware/vbios.c/.h` to
  `src/vm/profile/default_profile/firmware/bios.c/.h`.

The transitional retained alias and lifecycle names such as `vbios`,
`vbiosAddPost`, `vbiosAddInt`, `vbiosInit`, `vbiosReset`, `vbiosRefresh`, and
`vbiosFinal` remain direct compatibility names. No BIOS/POST construction,
interrupt-vector registration, reset-vector behavior, QDX registration,
fixture boot behavior, Console grammar, or debugger behavior changed.

## Verification

- Configured with MinGW-w64 GCC/CMake/Ninja.
- Built focused targets:
  `nxvm-vm-bios-authority-smoke`, `nxvm-default-profile-smoke`,
  `nxvm-full-pc-profile-smoke`, `nxvm-vm-full-authority-closure-smoke`,
  `verify-dependency-dag`, `verify-live-machine-authority`,
  `nxvm-product-console-smoke`, and `nxvm-0-5-0050`.
- Ran:
  - `nxvm-vm-bios-authority-smoke`: `M5:T38:S1:BIOS-AUTHORITY:OK`.
  - `nxvm-default-profile-smoke`: exit status 0.
  - `nxvm-full-pc-profile-smoke`: exit status 0.
  - `nxvm-vm-full-authority-closure-smoke`: exit status 0.
  - `nxvm-product-console-smoke`: `M5:T6:S1:NXVM-CONSOLE:OK`.
- Ran the full CMake build after focused gates.
- `verify-dependency-dag`: zero known migration edges.
- `verify-live-machine-authority`: no legacy full-PC storage definitions.

## Artifact

- Path: `build/output/nxvm_0_5_0050.exe`.
- SHA-256:
  `A9BF9585CB0BEA986C0A2FF39735C57CDB3EB32480384BCD6FBEC8D03FB7BBED`.
- Banner:

```text
Neko's x86 Virtual Machine [0.5.0050]
Copyright (c) 2012-2014 Neko.
```

Piped `EXIT` reached the retained Console and returned zero.
