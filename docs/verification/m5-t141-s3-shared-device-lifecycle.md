# M5 T141 S3: Core Shared-Device Lifecycle

## Result

`core_machine_create()` now initializes VADP, KBC, DMA, PIT, PIC, and their
fixed generic PIT-to-PIC binding after CPU/RAM/port preparation. Reset and run
refresh always service those same objects. `core_machine_destroy()` is their
sole finalizer after VM composition has finalized VM-only FDC/CMOS/media and
firmware state.

VM composition retains BIOS POST and interrupt registration, FDC-to-core device
binding, and VM-only lifecycle. It no longer initializes, resets, refreshes,
or finalizes a core shared device. VDM uses the same unconditional core path.

## Verification

Production source scans find no `core_machine_enable_shared_devices`,
`shared_devices_enabled`, or core shared-device lifecycle call below `src/vm`
or `src/vdm`. Windows GCC built and ran the lifecycle, VDM-minimal,
two-session, debugger-target, retained Console, and FDD DOS-prompt gates:

```text
M3:T2:S1:LIFECYCLE:OK
M5:T94:S1:VDM-SESSION-ISOLATION:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T14:S3:VM-DEBUG-TARGET:OK
M5:T96:S1:CONSOLE-LIFECYCLE:OK
M5:T70:S2:DOS-PROMPT:OK
```

Task artifact: `build/output/nxvm_0_5_0141.exe`, a local developer artifact.
Its banner is `Neko's x86 Virtual Machine [0.5.0141]`; SHA-256 is
`E40F99CB8083E0F9847C567109DD8217C7D6DFE92D419B3B6BB4A405B58D910B`.
