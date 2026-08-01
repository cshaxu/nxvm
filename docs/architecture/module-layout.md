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
  nxvm_baseline/
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

Temporary adapters are classified by their actual owner and moved to `core`,
`vm`, or `vdm`; no top-level adapter root remains at source-root closure.
`nxvm_baseline` is the immutable imported reference and independently buildable
regression target; it cannot supply a final-product source after M5 T13.

## Dependencies

`vm/*` and `vdm/*` may depend on `core/*` and their own product root.
`core/product` may depend on abstract `core/machine` and `core/platform`;
`core/platform` may depend on the host OS but not guest state. Profiles wire
only their own product components. Forbidden dependencies are core-to-VM/VDM
policy, core-to-concrete product UI, platform-to-guest-state, VM-to-VDM,
VDM-to-VM, and profile-to-foreign-product implementation. All guest-state
mutation occurs on the machine execution thread at a command boundary.

## Migration Rule

The current roots `app`, `adapters`, `dos`, `firmware`, `integration`,
`machine`, `platform`, `product`, `products`, and `runtime` are migration
sources only.
Their contents move in small buildable slices, with includes and CMake repaired
immediately after each move, and their directories are deleted when empty. A
runtime file moves to `core/product/runtime` only
when it serves both products without VM/VDM policy; VM startup/Console/profile/
media logic moves to `vm/product`, and VDM launch/profile/display/Console/
cancellation logic moves to `vdm/product`. Stop for an owner decision when a
file cannot be classified from its actual dependencies. The immutable legacy
reference root is renamed from `nxvm-baseline` to `nxvm_baseline` without
mixing it with migrated product code. No new source may be added to a
migration-source root.
