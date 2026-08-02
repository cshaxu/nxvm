# M5 T51 S1 Verification: Interface And Provider Naming

## Change

- Split `core/machine/block_interface.h` so injected block callbacks live in
  `core/machine/block_provider.h`.
- Split `core/product/wait.h` so injected wait callbacks live in
  `core/product/wait_provider.h`.
- Renamed core firmware public names from `nxvm_firmware*` /
  `NXVM_FIRMWARE*` to `core_machine_firmware*` /
  `CORE_MACHINE_FIRMWARE*`.
- Added the task artifact target for `0.5.0051`.

## Artifact

- Path: `build/output/nxvm_0_5_0051.exe`
- SHA256:
  `8F0368381CA10703134C7CDC2093BA253ECBAC168FDEC816AD846E6B38DAFF4D`
- Banner:

```text
Neko's x86 Virtual Machine [0.5.0051]
Copyright (c) 2012-2014 Neko.
```

## Evidence

- GCC CMake/Ninja full build: passed.
- `nxvm-core-product-wait-smoke`: `M5:T14:S3:CORE-PRODUCT-WAIT:OK`
- `nxvm-firmware-smoke`: `M5:T4:S1:FIRMWARE:OK`
- `nxvm-default-profile-firmware-smoke`: `M5:T4:S2:PC-AT-FIRMWARE:OK`
- `nxvm-vm-full-authority-closure-smoke`: passed.
- `nxvm-product-console-smoke`: `M5:T6:S1:NXVM-CONSOLE:OK`
- `verify-dependency-dag`: zero known migration edges.
- `verify-live-machine-authority`: no legacy full-PC storage definitions.
- `echo EXIT | build/output/nxvm_0_5_0051.exe`: `EXITCODE=0`.

## Behavior Boundary

No provider lifetime, callback order, Console text/grammar, debugger behavior,
boot/reset ordering, media behavior, or guest-visible behavior changed.
