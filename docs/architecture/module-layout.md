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
    product/session/
  vm/
    main.c
    composition/session/
    {machine,platform,product,profile}/
    profile/default_profile/firmware/
  vdm/
    composition/session/
    {machine,platform,product,profile}/
    profile/dos_minimal_profile/
```

Headers stay beside implementations. A header consumed outside its owning
module is named `<subject>_interface.h` and contains only the needed contract.
An owning module may use `<subject>.h` for private instance layout and internal
helpers. `machine.h`, for example, is private to `core/machine`; its public
handle contract is `machine_interface.h`. Device models are flat files unless
they become real multi-file subsystems.
`vm/main.c` is the current `nxvm.exe` entry point. `vdm/main.c` is the future
`ntvdm64.exe` entry point and must remain thin when introduced; until then,
VDM code is limited to design and smoke-test composition.

The concrete rules for distinguishing a private implementation, an exposed
interface, an injected provider, and a session-owned registry are defined in
[Core Machine Instance Design](core-machine-instance-design.md). That document
also owns the forward migration away from selected-session globals.

## Foundation Units

`src/type.h` is the sole common type header and legacy diagnostic foundation.
It defines `ntvdm64_status`, retained NXVM numeric aliases such as `t_nubit8`
and `t_bool`, fixed-width compatibility typedefs, common bit/constant helpers,
and product-neutral legacy C-runtime and trace primitives needed by more than
one module. `src/type.c` owns their non-inline implementations. All modules
may include `type.h`; this is a foundation-unit dependency, not a dependency
between product forms or modules.

The all-uppercase C-runtime wrappers in `type.*` are intentional retained
NXVM vocabulary: `LOCALTIME`, `STRCAT`, `STRCPY`, `STRTOK`, `STRCMP`, `STRLEN`,
`PRINTF`, `FPRINTF`, `SPRINTF`, `FOPEN`, `FCLOSE`, `FREAD`, `FWRITE`, `FGETS`,
`MALLOC`, `FREE`, `MEMSET`, `MEMCPY`, and `MEMCMP`. They are not legacy aliases
and are excluded from naming-remediation scans.

Each product module owns its compile-time `PRODUCT_NAME`; the shared
core-product banner helper supplies version, copyright, and build time. No
module contract contains an ABI version, timestamp, or compatibility probe;
the repository is one synchronously built system.

Public C symbols use their source ownership path: `core_machine_*`,
`core_platform_*`, `core_product_*`, `vm_machine_*`, `vm_platform_*`,
`vm_product_*`, `vm_profile_*`, and their VDM counterparts. Root composition
exports its concrete session as `vm_session_*` or `vdm_session_*`. Internal
composition helpers remain private to their product root.

Composition implementation and private headers live under
`vm/composition/` or `vdm/composition/`. The product root owns that directory;
it is not a fourth module and does not relax the directed module dependency
rules. Product entry points remain directly under `vm/main.c` and, when added,
`vdm/main.c`.

Common product-session tooling belongs in `core/product/session/`: opaque
entry registration, selection, copied snapshots, shared commands, and explicit
provider contracts. It never creates or understands a concrete VM/VDM session.
VM and VDM root composition own their concrete session construction, provider
implementation, selected-item adaptation, and teardown under
`composition/session/`. Product UI receives copied snapshots and calls the
core-product contract; it never owns or caches a selected session pointer or
ID. `core/composition/` is forbidden because it would be an accidental second
product assembly layer. This rule is realized by
[`planning/m5-product-session-management.md`](../planning/m5-product-session-management.md).

Approved compact lexical families are also retained: `kbc`, `vadp`,
`win32app`, `win32con`, `linuxapp`, `linuxcon`, `w32*`, `xasm32`, `aasm`,
`dasm`, and `debug`. The default-profile QDX firmware hacks remain `qdx`,
`qdcga`, and `qdkeyb`. Retention of those terms does not exempt their enclosing
public APIs from their owner prefix.

## Interface Naming

Public contracts are explicit rather than inferred from include paths.

- A cross-module machine contract uses a name such as `machine_interface.h`,
  `memory_interface.h`, `port_interface.h`, `debug_interface.h`,
  `trace_interface.h`, or `lifecycle_interface.h`.
- Public types and functions use the source-owner prefix, such as
  `core_machine_*`, `vm_machine_*`, or `vdm_platform_*`.
- An implementation supplied by another owner is a `*_provider`; a callback
  type also ends in `_provider`, and its installation function ends in
  `_bind_provider` or `_install_provider`.
- The owning machine contract fixes callback order, failure handling, and
  lifetime. A profile, platform, or product may supply a provider but may not
  alter those rules.

Where a capability has multiple selectable services or providers, its
`*_registry_interface.h` owns registration, conflict handling, lookup, and
freeze. It is separate from the provider contract; a descriptor alone is not a
provider.

`*_interface.h` marks an internal repository dependency contract, not a
versioned SDK or ABI promise. Legacy compatibility aliases are temporary
implementation detail, never a new public contract.

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

A `core/platform` host-surface context contains only a surface kind and an
opaque native handle. A host resource that cannot be shared, such as a process
terminal, is represented by a caller-owned `core_platform_host_surface_lease`.
It atomically names one explicit composition owner; acquire by a second owner
fails, and only that owner can release it. The contract has no guest state,
renderer state, process singleton, or product policy. VM and VDM composition
choose whether to create a context, acquire a lease, or reject a request.

The retained hardware debugger command language, prompt, help, and text
presentation are shared product UX and belong in `core/product/debug`. Its
core-owned debug target declares the machine effects it needs; VM and VDM root
composition bind that target to their respective machines. Product forms may
add capability-specific commands, but they do not fork the common debugger UI.

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
controllers such as FDC/HDC/FDD/HDD. `vm/platform` owns only full-machine
policy adapters; concrete host facilities shared with VDM belong in
`core/platform/{win32,linux}`. `vm/product` owns retained NXVM user experience: Console, hardware
debugger UX, media commands, and presentation policy. `vm/profile` owns VM
topology, boot policy, ROM assets, and declarative firmware-provider metadata.
The `vm/` root composition selects that profile, creates the providers, and
binds their callbacks and lifetime. The retained Console receives a
product-owned command target from that composition; it owns parsing, text, and
command UX while the target owns machine, platform, debug, and media effects.
Profile-specific firmware code is allowed
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

## Session Readiness

An implemented module is session-ready only when every mutable datum is
session-owned, execution-thread-owned, caller-owned invocation state, or an
explicit process-exclusive host lease. Immutable tables and descriptors may be
shared. A process-global, `_Thread_local`, or implicit-current-object selector
is not an acceptable production-state owner. The authoritative checklist,
priority order, and closure tasks are in
[`planning/m5-session-readiness.md`](../planning/m5-session-readiness.md).
An undocumented mutable file-static object is never an acceptable substitute
for a session context or host lease.

## M5 Convergence

M5 removed the prior `app`, `adapters`, `dos`, `firmware`, `integration`,
`machine`, `nxvm-baseline`, `platform`, `product`, `products`, and `runtime`
source roots. Only `core`, `vm`, and `vdm` directories, plus the root
foundation units `type.*`, may receive source files.

The source-root, naming, and selected-session-authority plans are complete and
archived under `history/m5/planning/`. T64 through T73 removed the selected
live-object aliases, but did not eliminate every process-global facade or unify
the minimal `core_machine` path with the real NXVM executor. The current M5
convergence plan first removes those facades, then makes `core_machine` the
true guest executor while VM retains its outer product loop. Shared concrete
Win32/Linux host providers move to `core/platform` only when proven
mechanism-only; VM and VDM policies remain bound by root composition.
