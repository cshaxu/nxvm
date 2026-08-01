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
    composition.{c,h}
    {machine,platform,product,profile}/
    profile/default_profile/firmware/
  vdm/
    main.c
    composition.{c,h}
    {machine,platform,product,profile}/
    profile/dos_minimal_profile/
```

Headers stay beside implementations. Device models are flat files unless they
become real multi-file subsystems; private implementation headers use `_impl.h`.
`vm/main.c` is the `nxvm.exe` entry point and `vdm/main.c` is the
`ntvdm64.exe` entry point. Each remains thin and enters its product-form root
composition in `vm/composition.*` or `vdm/composition.*`.

## Foundation Units

`src/type.h` is the sole common type header and legacy diagnostic foundation.
It defines standard project types such as `BOOL`, `SIZE_T`, fixed-width
aliases, `STATUS`, and the stable `STATUS_*` result constants, together with
the product-neutral legacy C-runtime and trace primitives needed by more than
one module. `src/type.c` owns their non-inline implementations. All modules
may include `type.h`; this is a foundation-unit dependency, not a dependency
between product forms or modules.

`src/version.h` and `src/version.c` are the sole version and build-identity
source. They provide product Console banner identity and build timestamp data.
No module contract contains an ABI version, timestamp, or compatibility probe;
the repository is one synchronously built system.

Public C symbols use their source ownership path: `core_machine_*`,
`core_platform_*`, `core_product_*`, `vm_machine_*`, `vm_platform_*`,
`vm_product_*`, `vm_profile_*`, and their VDM counterparts. Root composition
uses `vm_composition_*` or `vdm_composition_*`.

## Ownership

`core/machine` contains product-neutral guest mechanics: CPU/instructions,
RAM, ports, interrupts, shared execution support, reusable
PIC/PIT/DMA/CMOS/keyboard/video/block models, trace/debug state, and the
firmware-service registry. The registry describes POST, ROM, and interrupt
services but contains no PC/AT handler, ROM image, product policy, or host OS
call.

`core/platform` contains product-neutral host-capability contracts and shared
host facilities. It never mutates guest state. `core/product` contains generic
command, debug/trace, registry, result, assembler, and disassembler tooling,
but no session composition, VM Console, VDM CLI, profile, boot/media, or
host-policy decision. It may expose a product-neutral provider contract, such
as the bound wait service used by retained firmware and debugger paths; root
composition owns the host implementation and its lifetime. Its runtime
infrastructure lives in `core/product/runtime`; there is no top-level runtime
module.

Machine snapshots and platform frames are distinct contracts. A product-machine
snapshot may contain text cells, attributes, geometry, cursor, generation, and
machine-private diagnostics, and may embed a core text snapshot; it remains a
machine type. A product-platform frame contains only copied host-facing
presentation data and must not embed, point at, or name a machine snapshot
type. The corresponding VM or VDM root composition is the sole source that may
include both contracts: at a defined execution boundary it converts the
snapshot to a frame and submits it. Product-private diagnostics such as
DOS-minimal PIT state or pending keyboard IRQ remain in that product's machine
module and require a separate diagnostic contract if they must be exposed.

Ownership is determined by reuse, not by abstraction level or the source's
current directory. Any logic used by both products belongs in the matching
`core/{machine,platform,product}` owner, including concrete host code:
shared Win32 and Linux providers live in `core/platform/win32` and
`core/platform/linux`; platform-neutral platform code lives directly in
`core/platform`. A product-only implementation belongs under its `vm/*` or
`vdm/*` counterpart. The same rule applies to machine and product code.

`vm/machine` owns boot/reset sequencing, execution-loop glue, and VM-only
controllers such as FDC/HDC/FDD/HDD. `vm/platform` owns full-machine host
providers. `vm/product` owns retained NXVM user experience: Console, hardware
debugger UX, media commands, and presentation policy. `vm/profile` owns VM
topology, boot policy, ROM assets, and declarative firmware-provider metadata.
The `vm/` root composition selects that profile, creates the providers, and
binds their callbacks and lifetime. Profile-specific firmware code is allowed
only as an override provider against a public core contract; it does not create
the machine or call a sibling module directly.

`vdm/machine` owns the DOS loader, PSP, environment, DTA, handles, paths, DOS
devices/services, errors, and program exit. `vdm/platform` owns app-runner
host providers for parent-Console protection, cancellation, filesystem
containment, and presentation/input. `vdm/product` owns `ntvdm64 run`, launch
parameters, VDM debugging UX, display/Console policy, and cancellation UX.
`vdm/profile` owns declarative DOS memory/service/device policy and any
firmware-service subset. The `vdm/` root composition selects that profile and
binds machine, platform, product, and teardown.

Temporary adapters are classified by their actual owner and live under `core`,
`vm`, or `vdm`; no top-level adapter root remains. The imported
`nxvm-baseline` tree was fully migrated and deleted. Git history and the
recorded M1 snapshot preserve provenance; it is not a source root.

## Dependency Model

The required architecture is a directed acyclic graph, not a collection of
mutually aware subsystems. No module may reach sideways to a sibling module
and no lower module may depend on a product form. The product-form root
composition is the only permitted integration point.

```text
core/machine      core/platform      core/product
     (independent libraries with public provider contracts)

vm/machine   -> core/machine          vdm/machine   -> core/machine
vm/platform  -> core/platform         vdm/platform  -> core/platform
vm/product   -> core/product          vdm/product   -> core/product
vm/profile   -> core contracts        vdm/profile   -> core contracts
       \       |       /                       \       |       /
        +------v------+                         +------v------+
             vm/compose                              vdm/compose
```

`core/machine` is the leaf for mutable guest state and guest-domain contracts.
It must not include `core/platform`, `core/product`, `vm/*`, or `vdm/*`.
`core/platform` is the leaf for host-capability contracts and shared host
providers. It must not mutate guest state or include `core/product`, `vm/*`,
or `vdm/*`; it also does not include `core/machine` or `core/product`.
Platform-facing frames and events are platform contracts. Only the relevant
product root composition translates a machine-owned snapshot into such a frame
when required; no platform header may name a machine snapshot type.

`core/product` contains reusable product tooling only: generic command,
registry, trace, debug, assembler, and disassembler facilities. It may depend
only on its own public callback contracts; it may not select a product, own a
product profile, instantiate a VM/VDM session, or include `core/machine`,
`core/platform`, `vm/*`, or `vdm/*`. A root composition adapts a concrete
machine or platform provider to a generic product-tool target.

Within either product form, `machine`, `platform`, `product`, and `profile`
are peer providers. They may depend on matching core contracts but must not
include one another. A profile is declarative data and provider metadata, not
a machine constructor. It may contain a profile-specific ROM or firmware
override only through a public core callback contract. The `vm/` or `vdm/`
root composition may depend on all four modules and on `core/*`; it chooses a
profile, creates the machine, binds platform capabilities and product UX, and
owns execution-loop policy and teardown. Adapters which translate input,
display snapshots, or callbacks belong to that root composition, rather than
creating a `machine <-> platform` dependency. Root composition repeatedly
drives bounded synchronous machine quanta; it owns host threads, wall-clock
watchdogs, pacing, and product exit policy.

The three core modules have zero compile-time dependency on one another.
Cross-domain data is carried through public provider contracts and translated
by a product-form root composition; a platform never imports a machine snapshot
type. All guest-state mutation occurs on the machine execution thread at a
command boundary.

Forbidden dependencies are any core-to-VM/VDM path, any VM-to-VDM or
VDM-to-VM path, sibling module includes within `vm` or `vdm`, profile-to-product
construction, platform-to-guest-state mutation, and all dependency cycles.
The build-target graph follows the same rules: a target may not conceal a
forbidden source edge through an aggregate library.

## Migration Closure

M5 removed the prior `app`, `adapters`, `dos`, `firmware`, `integration`,
`machine`, `nxvm-baseline`, `platform`, `product`, `products`, and `runtime`
source roots. Only `core`, `vm`, and `vdm` directories, plus the root
foundation units `type.*` and `version.*`, may receive source files.

The retained NXVM executor still has explicitly recorded legacy direct calls
between moved owners, such as an instruction stop path and a VM sleep service.
They preserve behavior during this source-preserving migration and are not new
module APIs. M5 dependency-governance work removes them without changing the
retained Console, debugger, boot sequence, or media behavior.
