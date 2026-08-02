# M5 T53 S1 Verification: vglobal Retirement

## Change

- Moved retained NXVM numeric aliases, pointer helpers, bit/constant helpers,
  BCD helpers, and `ExecFun` from `src/core/machine/vglobal.h` to
  `src/type.h`.
- Kept `src/core/machine/vglobal.h` as a transitional include for retained
  device code.
- Added the task artifact target for `0.5.0053`.

## Artifact

- Path: `build/output/nxvm_0_5_0053.exe`
- SHA256:
  `77058F8454DFC41CB007D20FE0BBEC92CA19BF30CA658EF889A89A4021DE9104`
- Banner:

```text
Neko's x86 Virtual Machine [0.5.0053]
Copyright (c) 2012-2014 Neko.
```

## Evidence

- GCC CMake/Ninja full build: passed.
- `nxvm-core-contract-smoke`: `M3:T1:S1:CORE-CONTRACT:OK`
- `nxvm-core-machine-instance-smoke`: `M3:T1:S2:MACHINE-INSTANCE:OK`
- `nxvm-vm-full-authority-closure-smoke`: passed.
- `nxvm-product-console-smoke`: `M5:T6:S1:NXVM-CONSOLE:OK`
- `ntvdm64-version-smoke`: `M5:T47:S1:VERSION-FOUNDATION:OK`
- `verify-dependency-dag`: zero known migration edges.
- `verify-live-machine-authority`: no legacy full-PC storage definitions.
- `echo EXIT | build/output/nxvm_0_5_0053.exe`: `EXITCODE=0`.

## Behavior Boundary

No Console text/grammar, debugger behavior, boot/reset ordering, media
behavior, type-size setting, provider lifetime, or guest-visible behavior
changed.
