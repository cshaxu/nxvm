# Module Layout

## Authority

This is the forward-looking source-layout authority. Historical M1 through M5
records retain evidence, not ownership rules. Source moves use `git mv`; copied
or independently rewritten NXVM implementations are prohibited.

## Topology

```text
src/
  base/
  machine/{core,vm,vdm}/
  platform/core/{win32,linux}/
  platform/vm/{win32,linux}/
  platform/vdm/{win32,linux}/
  product/{core,vm,vdm}/
  profile/vm/{default_profile,pc110_profile,...}/
  profile/vdm/{dos_minimal_profile,...}/
  integration/
```

Headers stay beside implementations. Device models are flat files unless they
become real multi-file subsystems.

## Ownership

`machine/core` contains shared CPU/instructions, RAM, bus, interrupts,
execution loop, trace/debug state, and reusable PIC/PIT/DMA/CMOS/keyboard/video
and block models. It knows no host OS, product UX, DOS ABI, or profile name.
`machine/vm` contains boot/reset sequencing and VM-only hardware; the current
FDC/HDC/FDD/HDD stack starts there until proven reusable. `machine/vdm` contains
the DOS loader, PSP, environment, DTA, handles, paths, DOS devices/services,
errors, and program exit. VM and VDM share the core execution loop.

`platform/core` contains host contracts and shared Win32/Linux facilities.
`platform/vm` adds only full-machine presentation/input routing; `platform/vdm`
adds only app-runner process lifetime, parent-console protection, cancellation,
and filesystem containment. Platform never mutates guest state.

`product/core` owns session/registry/result/version and shared debugger,
breakpoint, step, trace, assembler, and disassembler logic. `product/vm` owns
the exact NXVM Console and VM UX; `product/vdm` owns `ntvdm64 run` and VDM UX.

There is no `profile/core`. `profile/vm` describes hardware topology, wiring,
firmware/ROM/POST/CMOS defaults and boot policy; it may supply a capability
subset to either product. `profile/vdm` describes DOS loader, memory layout,
service level, DOS-device policy, and firmware-service subset. Profiles select
and wire components; they never implement CPU, devices, DOS, or host calls.

## Dependencies

`machine/vm|vdm -> machine/core`; `product -> abstract machine/platform`;
`profile -> abstract machine/product contracts`; `platform -> host OS`.
Forbidden: core-to-platform/product/profile, platform-to-guest-state,
profile-to-device implementation, and product-to-concrete host API.
