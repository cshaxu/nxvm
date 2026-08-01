# Module Layout

## Authority

This is the forward-looking source-layout authority. Historical M1 through M5
records retain evidence, not ownership rules. Source moves use `git mv`; copied
or independently rewritten NXVM implementations are prohibited.

## Terms

A **product form** is one of `core`, `vm`, or `vdm`. A **module** is one of
`machine`, `platform`, `product`, or `profile`. `core` has no profile module:
it is the shared foundation. `vm` and `vdm` are separate products, not layers
above one another.

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

Shared presentation contracts carry only presentation data: text cells,
attributes, geometry, cursor, generation, and timestamps. Product-private
machine diagnostics such as DOS-minimal PIT state or pending keyboard IRQ state
remain in that product's machine module and require a separate product
diagnostic contract if they must be exposed. A product-private snapshot may
embed the shared core text snapshot; its platform adapter copies that child
object without translating product diagnostics into presentation data.

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

## Dependency Model

The required architecture is a directed acyclic graph, not a collection of
mutually aware subsystems. No module may reach sideways to a sibling module
and no lower module may depend on a product form. Product composition is the
only permitted integration point.

```text
core/machine  <--- core/platform (immutable public data contracts only)
      ^                    ^
      |                    |
core/product --------------+

vm/machine   -> core/machine          vdm/machine   -> core/machine
vm/platform  -> core/platform         vdm/platform  -> core/platform
vm/profile   -> core contracts        vdm/profile   -> core contracts
       \             |                       \             |
        +------------+                        +------------+
                     v                                     v
                 vm/product                            vdm/product
```

`core/machine` is the leaf for mutable guest state and guest-domain contracts.
It must not include `core/platform`, `core/product`, `vm/*`, or `vdm/*`.
`core/platform` is the leaf for host-capability contracts and shared host
providers. It must not mutate guest state or include `core/product`, `vm/*`,
or `vdm/*`. It may consume an immutable public value type owned by
`core/machine`, such as a copied text snapshot. This single one-way type
dependency avoids duplicating a guest-domain representation; it is not a
platform-to-machine control path.

`core/product` contains reusable product tooling only: generic command,
registry, trace, debug, assembler, and disassembler facilities. It may depend
on the public contracts of `core/machine` and `core/platform`, but it may not
select a product, own a product profile, instantiate a VM/VDM session, or
include `vm/*` or `vdm/*`.

Within either product form, `machine`, `platform`, and `profile` are peer
providers. They may depend on matching core contracts but must not include one
another. A profile is declarative data and provider metadata, not a machine
constructor. `vm/product` or `vdm/product` is the sole composition root: it
may depend on all modules in its own product form and on `core/*`, choose a
profile, create the machine, bind platform capabilities, and own teardown.
Product adapters which translate input, display snapshots, or callbacks belong
there, rather than creating a `machine <-> platform` dependency.

Thus zero *mutual* dependency is mandatory. Completely zero dependency among
the three core modules is neither necessary nor desirable while a platform
must carry a typed immutable machine snapshot; the permitted edge is narrow,
one-way, and has no lifecycle or callback authority. All guest-state mutation
occurs on the machine execution thread at a command boundary.

Forbidden dependencies are any core-to-VM/VDM path, any VM-to-VDM or
VDM-to-VM path, sibling module includes within `vm` or `vdm`, profile-to-product
construction, platform-to-guest-state mutation, and all dependency cycles.
The build-target graph follows the same rules: a target may not conceal a
forbidden source edge through an aggregate library.

## Migration Closure

M5 removed the prior `app`, `adapters`, `dos`, `firmware`, `integration`,
`machine`, `nxvm-baseline`, `platform`, `product`, `products`, and `runtime`
source roots. Only `core`, `vm`, and `vdm` may receive source files.

The retained NXVM executor still has explicitly recorded legacy direct calls
between moved owners, such as an instruction stop path and a VM sleep service.
They preserve behavior during this source-preserving migration and are not new
module APIs. M5 dependency-governance work removes them without changing the
retained Console, debugger, boot sequence, or media behavior.
