# Module Layout

## Authority

This is the forward-looking source-layout authority. Historical M1 through M5
records retain evidence, not ownership rules. Source moves use `git mv`; copied
or independently rewritten NXVM implementations are prohibited.

## Topology

```text
src/
  core/{machine,platform,product}/
  vm/
    main.c
    {machine,platform,product,profile}/
    profile/default_profile/firmware/
  vdm/
    main.c
    {machine,platform,product,profile}/
    profile/dos_minimal_profile/
```

Headers stay beside implementations. Device models are flat files unless they
become real multi-file subsystems; private implementation headers use `_impl.h`.
`vm/main.c` is the `nxvm.exe` entry point and `vdm/main.c` is the
`ntvdm64.exe` entry point.

## Ownership

`core/machine` contains product-neutral guest mechanics: CPU/instructions,
RAM, ports, interrupts, shared execution support, reusable
PIC/PIT/DMA/CMOS/keyboard/video/block models, trace/debug state, and the
firmware-service registry. The registry describes POST, ROM, and interrupt
services but contains no PC/AT handler, ROM image, product policy, or host OS
call.

`core/platform` contains product-neutral host-capability contracts and shared
host facilities. It never mutates guest state. `core/product` contains shared
session lifecycle, registry composition, abstract command boundaries, generic
debug/trace coordination, result, assembler, and disassembler infrastructure,
but no VM Console, VDM CLI, profile, boot/media, or host-policy decision. Its
runtime infrastructure lives in `core/product/runtime`; there is no top-level
runtime module.

Ownership is determined by reuse, not by abstraction level or the source's
current directory. Any logic used by both products belongs in the matching
`core/{machine,platform,product}` owner, including concrete host code:
shared Win32 and Linux providers live in `core/platform/win32` and
`core/platform/linux`; platform-neutral platform code lives directly in
`core/platform`. A product-only implementation belongs under its `vm/*` or
`vdm/*` counterpart. The same rule applies to machine and product code.

`vm/machine` owns boot/reset sequencing, execution-loop glue, and VM-only
controllers such as FDC/HDC/FDD/HDD. `vm/platform` owns full-machine
input/presentation routing. `vm/product` owns the retained NXVM Console,
hardware debugger, media workflow, full-machine startup, VM profile selection,
and VM composition. `vm/profile` owns VM
topology and boot policy; the built-in PC/AT BIOS, POST, ROM, QDX handlers, and
default CMOS wiring live in `vm/profile/default_profile/firmware`.

`vdm/machine` owns the DOS loader, PSP, environment, DTA, handles, paths, DOS
devices/services, errors, and program exit. `vdm/platform` owns app-runner
process lifetime, parent-Console protection, cancellation, filesystem
containment, and VDM presentation/input routing. `vdm/product` owns
`ntvdm64 run`, program-launch parameters, VDM debugging UX, display/Console
policy, cancellation policy, execution-profile selection, and composition.
`vdm/profile` owns DOS
memory/service/device policy and any firmware-service subset.

Temporary adapters are classified by their actual owner and live under `core`,
`vm`, or `vdm`; no top-level adapter root remains. The imported
`nxvm-baseline` tree was fully migrated and deleted. Git history and the
recorded M1 snapshot preserve provenance; it is not a source root.

## Dependencies

`vm/*` and `vdm/*` may depend on `core/*` and their own product root.
`core/product` may depend on abstract `core/machine` and `core/platform`;
`core/platform` may depend on the host OS but not guest state. Profiles wire
only their own product components. Forbidden dependencies are core-to-VM/VDM
policy, core-to-concrete product UI, platform-to-guest-state, VM-to-VDM,
VDM-to-VM, and profile-to-foreign-product implementation. All guest-state
mutation occurs on the machine execution thread at a command boundary.

## Migration Closure

M5 removed the prior `app`, `adapters`, `dos`, `firmware`, `integration`,
`machine`, `nxvm-baseline`, `platform`, `product`, `products`, and `runtime`
source roots. Only `core`, `vm`, and `vdm` may receive source files.

The retained NXVM executor still has explicitly recorded legacy direct calls
between moved owners, such as an instruction stop path and a VM sleep service.
They preserve behavior during this source-preserving migration and are not new
module APIs. Any later decoupling must be designed and approved separately.
