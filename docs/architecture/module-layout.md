# Module Layout

## Authority

This is the forward-looking source-layout authority. Historical M1 through M5
records retain evidence, not ownership rules. Source moves use `git mv`; copied
or independently rewritten NXVM implementations are prohibited.

## Terms

A **component** is one of `core`, `vm`, `mantle`, `dos`, or `vdm`. A **module**
is one of `machine`, `platform`, `product`, or `profile`. `core` has no profile
module: it is the shared foundation. `dos` has no dependency on another
component. `vm` is the NXVM product; `mantle` is shared VDM composition; `vdm`
is the NXVDM product shell over mantle and dos.

## Current And Target Topology

The current tracked tree and the target component topology are deliberately
separate. The target diagram is an architecture contract, not an inventory of
directories that a fresh clone must contain.

### Current Tracked Source Tree (M5)

```text
src/
  type.*
  core/{utils,machine,platform,product}/
  vm/
    main.c
    composition/session/
    {machine,platform,product,profile}/
    profile/default_profile/firmware/
  vdm/
    composition/
    machine/
```

`vdm/` is the retained M3 non-runnable skeleton and smoke-fixture source. It
does not define `nxvdm.exe`, an NXVDM CLI, mantle, or the owned DOS backend.
`src/mantle/`, `src/dos/`, `src/vdm/main.c`, and the remaining VDM modules do
not yet exist in the tracked tree.

### Target Component Topology (M6--M8)

```text
src/
  core/{utils,machine,platform,product}/
    product/session/
  vm/
    main.c
    composition/session/
    {machine,platform,product,profile}/
    profile/default_profile/firmware/
  mantle/
    composition/session/
    {machine,platform,product}/
  dos/
    {machine,platform,product,profile}/
  vdm/
    main.c
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
`nxvdm.exe` entry point and remains thin: it selects NXVDM product UX and binds
mantle to dos. `mantle` owns reusable VDM session composition; `dos` remains
independent and does not include mantle or core headers.

The concrete rules for distinguishing a private implementation, an exposed
interface, an injected provider, and a session-owned registry are defined in
[Core Machine Instance Design](core-machine-instance-design.md). That document
also owns the forward migration away from selected-session globals.
The current symbol-level migration baseline for the remaining public-surface
closure is [Core Machine Public-Surface Inventory](core-machine-public-surface-inventory.md);
it is a task design record, not an additional runtime contract.

## Foundation Units

`src/type.h` is the sole common type header and legacy diagnostic foundation.
It supplies product-neutral status, retained NXVM numeric aliases, common
bit/constant helpers, and shared runtime/trace primitives. `src/type.c` owns
their non-inline implementations. All modules may include `type.h`; this is a
foundation-unit dependency, not a dependency between product forms or modules.
The detailed C vocabulary and header boundary are defined only by
[C-Library Facade](c-library-facade.md).

`src/core/utils/` is the sole shared utility module. It contains only small,
product-neutral facilities and callback contracts that require neither guest
state nor host policy, such as an injected wait scope. It depends only on
`type-facade`; it may not include any `core/{machine,platform,product}` or
`vm/*`/`vdm/*` header. It is not a miscellaneous convenience layer: assembler,
disassembler, debugger UX, registries, device code, platform implementation,
and product policy remain with their named owners. Public symbols use the
`core_utils_*` prefix.

Each product module owns its compile-time `PRODUCT_NAME`; the shared
core-product banner helper supplies version, copyright, and build time. No
module contract contains an ABI version, timestamp, or compatibility probe;
the repository is one synchronously built system.

Public C symbols use their source ownership path: `core_machine_*`,
`core_platform_*`, `core_product_*`, `vm_machine_*`, `vm_platform_*`,
`vm_product_*`, `vm_profile_*`, `mantle_*`, `dos_*`, and `vdm_*`. Root
composition exports its concrete session as `vm_session_*` or
`mantle_session_*`. Internal composition helpers remain private to their
component root.

Composition implementation and private headers live under
`vm/composition/` or `mantle/composition/`. The component root owns that directory;
it is not a fourth module and does not relax the directed module dependency
rules. Product entry points remain directly under `vm/main.c` and `vdm/main.c`.

Common product-session tooling belongs in `core/product/session/`: opaque
entry registration, selection, copied snapshots, shared commands, and explicit
provider contracts. It never creates or understands a concrete VM/VDM session.
VM and mantle composition own their concrete session construction, provider
implementation, selected-item adaptation, and teardown under
`composition/session/`. Product UI receives copied snapshots and calls the
core-product contract; it never owns or caches a selected session pointer or
ID. `core/composition/` is forbidden because it would be an accidental second
product assembly layer. The completed implementation is summarized in
[M5 History](../history/m5.md).

A composition session's complete layout is private to its product root. Its
public header declares an opaque `vm_session` or `mantle_session` handle, stable
configuration, and lifecycle/operation contracts only. The complete struct and
construction helpers live in `session.h`, included only by that product
composition. Product peers receive only the opaque public contract. Tests may
include an implementation header and directly use its existing state when they
test that same module and introduce neither mirror state nor an alternate
runtime route; test-only facades are not required merely to hide an already
legitimate implementation pointer.

Approved compact lexical families are also retained: `kbc`, `vadp`,
`win32app`, `win32con`, `linuxapp`, `linuxcon`, `w32*`, `xasm32`, `aasm`,
`dasm`, and `debug`. The default-profile firmware names `qdcga` and `qdkeyb`
remain approved compact vocabulary. Retention of those terms does not exempt
their enclosing public APIs from their owner prefix.

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
host-policy decision. Product-neutral callback contracts shared by otherwise
independent owners belong in `core/utils`; root composition owns any concrete
host implementation and its lifetime.

A `core/platform` host-surface context contains only a surface kind and an
opaque native handle. A host resource that cannot be shared, such as a process
terminal, is represented by a caller-owned `core_platform_host_surface_lease`.
It atomically names one explicit composition owner; acquire by a second owner
fails, and only that owner can release it. The contract has no guest state,
renderer state, process singleton, or product policy. VM and mantle composition
choose whether to create a context, acquire a lease, or reject a request.

The retained hardware debugger command language, prompt, help, and text
presentation are shared product UX and belong in `core/product/debug`. Its
core-owned debug target declares the machine effects it needs; VM and mantle
composition bind that target to their respective machines. Product forms may
add capability-specific commands, but they do not fork the common debugger UI.

Machine snapshots and platform frames are distinct contracts. A product-machine
snapshot may contain text cells, attributes, geometry, cursor, generation, and
machine-private diagnostics, and may embed a core text snapshot; it remains a
machine type. A product-platform frame contains only copied host-facing
presentation data and must not embed, point at, or name a machine snapshot
type. The corresponding VM or mantle composition is the sole source that may
include both contracts: at a defined execution boundary it converts the
snapshot to a frame and submits it. Product-private diagnostics such as
DOS-minimal PIT state or pending keyboard IRQ remain in that product's machine
module and require a separate diagnostic contract if they must be exposed.

Ownership is determined by reuse, not by abstraction level or the source's
current directory. Small policy-free utilities and callback contracts belong in
`core/utils`; all other logic used by both products belongs in the matching
`core/{machine,platform,product}` owner, including concrete host code:
shared Win32 and Linux providers live in `core/platform/win32` and
`core/platform/linux`; platform-neutral platform code lives directly in
`core/platform`. A product-only implementation belongs under its `vm/*` or
`mantle/*`, `dos/*`, or `vdm/*` counterpart. The same rule applies to machine
and product code.

`core/machine` may provide optional profile-neutral controller mechanisms,
including MC146818-compatible RTC, FDC, and ATA PIO models, through explicit
port/IRQ/DMA/clock/media bindings. It owns their storage and
initialization/reset/finalization order. It never chooses PC/AT defaults,
media paths, boot policy, firmware bytes, or host policy. `vm/machine` owns
VM media backing objects and PC/AT-only policy; it does not retain a second
copy of a core controller. The `vm/` root composition selects a profile,
submits typed frozen configuration/provider bindings, owns provider lifetime
and the bounded product execution pump, and never duplicates the core
scheduler or borrows raw core state.
`core/platform` owns policy-free opaque host capability contracts and shared
Win32/Linux implementations. T271 currently supplies backing-resource, copied
input, and wait/cancellation surfaces. T282 audits those existing surfaces and
the host-surface lease: it must keep native handles opaque to core or move that
VM-specific contract to `vm/platform`; it does not admit file, directory,
stream, or sampled-clock APIs without a real consumer. `vm/platform` owns only full-machine
policy adapters: image selection, mount/persistence, Console/window behavior,
and PC/AT host choices. `vm/product` owns retained NXVM user experience: Console, hardware
debugger UX, media commands, and presentation policy. `vm/profile` owns VM
topology, boot policy, ROM assets, and declarative firmware-provider metadata.
The `vm/` root composition selects that profile, creates the providers, and
binds their callbacks and lifetime. The retained Console receives a
product-owned command target from that composition; it owns parsing, text, and
command UX while the target owns machine, platform, debug, and media effects.
Profile-specific firmware code is allowed only as an override provider against
an opaque core-invoked capability. It does not create the machine, borrow CPU,
RAM, port, controller, or executor storage, or call a sibling module directly.

VM profile declarations may describe only immutable capabilities, topology,
port/IRQ/DMA routes, controller/CMOS defaults, ROM-slot constraints,
firmware-hook metadata, media compatibility, and pure input mappings. They do
not own local media/ROM paths, a session, a machine/controller instance, a
thread, a platform/product handle, or a runtime selector. VM composition is
the sole constructor and translates one selected declaration into providers
before core freeze. An external-ROM manifest is future composition input; it
cannot become a profile asset or core contract.

`mantle/machine`, `mantle/platform`, and `mantle/product` are future reusable
VDM composition mechanism over core. A core-only mantle-shape fixture may
prove an admitted contract during M5, but it is not mantle implementation.
Trusted external research may inform a neutral mantle requirement, but mantle
knows no DOS ABI, external-runtime ABI, CLI grammar, path policy, protected
asset, or product exit policy.
`mantle/` root composition constructs the session and binds an admitted
DOS-runtime provider to core at defined execution boundaries. M5 may prove a
core-only second-consumer fixture, but it must not create `src/mantle/` before
the M6 entry gate.

`dos/machine` owns the independent DOS loader, PSP, environment, DTA, handles,
paths, DOS devices/services, errors, and program exit. `dos/platform`,
`dos/product`, and `dos/profile` may support that backend but cannot include
core, VM, mantle, or VDM headers.

The retained M3 `vdm/` skeleton directly uses core only to prove deterministic
machine and presentation contracts without guest media. It is a non-product
test fixture, never an alternative mantle/DOS path or an NXVDM executable.
M6/M8 admission must either replace it with the approved mantle-plus-DOS path
or retire it with its focused coverage moved to the owning component.

`vdm/platform` owns NXVDM host policy for parent-Console protection,
cancellation, filesystem containment, and presentation/input. `vdm/product`
owns `nxvdm run`, launch parameters, debugging UX, display/Console policy, and
cancellation UX. `vdm/machine` and `vdm/profile` are product adapters and
declarative policy only; they do not reimplement DOS or own composition.
`vdm/main.c` binds mantle to dos and applies NXVDM product policy.

Temporary adapters are classified by their actual owner and live under `core`,
`vm`, `mantle`, `dos`, or `vdm`; no top-level adapter root remains. The imported
`nxvm-baseline` tree was fully migrated and deleted. Git history and the
recorded M1 snapshot preserve provenance; it is not a source root.

## Dependency Model

The required architecture is a directed acyclic graph, not a collection of
mutually aware subsystems. No module may reach sideways to a sibling module
and no lower component may depend on a higher component. VM and mantle
composition are the only permitted integration points.

```text
             core/utils
                 |
core/machine      core/platform      core/product
     (independent libraries with public provider contracts)

vm/{machine,platform,product,profile} -> core contracts -> vm/compose -> nxvm.exe
mantle/{machine,platform,product}      -> core contracts -> mantle/compose
dos/{machine,platform,product,profile} -> no component dependency -> dos.dll
mantle.dll + dos.dll + vdm/{machine,platform,product,profile} -> nxvdm.exe
```

`core/machine` is the leaf for mutable guest state and guest-domain contracts.
It must not include `core/platform`, `core/product`, `vm/*`, `mantle/*`,
`dos/*`, or `vdm/*`.
`core/platform` is the leaf for host-capability contracts and shared host
providers. It must not mutate guest state or include `core/product`, `vm/*`,
`mantle/*`, `dos/*`, or `vdm/*`; it also does not include `core/machine` or
`core/product`.
Platform-facing frames and events are platform contracts. Only the relevant
product root composition translates a machine-owned snapshot into such a frame
when required; no platform header may name a machine snapshot type.

`core/product` contains reusable product tooling only: generic command,
registry, trace, debug, assembler, and disassembler facilities. It may depend
only on its own public callback contracts and `core/utils`; it may not select a
product, own a product profile, instantiate a VM/mantle session, or include
`core/machine`, `core/platform`, `vm/*`, `mantle/*`, `dos/*`, or `vdm/*`. A
root composition adapts
a concrete machine or platform provider to a generic product-tool target.

Within VM, DOS, or VDM, `machine`, `platform`, `product`, and `profile`
are peer providers. They may depend on matching core contracts but must not
include one another. A profile is declarative data and provider metadata, not
a machine constructor. It may contain a profile-specific ROM or firmware
override only through a public core callback contract. VM composition may
depend on its four modules and core; mantle composition may depend on its three
modules and core; VDM binds mantle to dos. Adapters which translate input,
display snapshots, or callbacks belong to that root composition, rather than
creating a `machine <-> platform` dependency. Root composition repeatedly
drives bounded synchronous machine quanta; it owns host threads, wall-clock
watchdogs, pacing, and product exit policy.

`core/utils` is below the three independent core modules. The three core
modules have zero compile-time dependency on one another; each may use the
strictly neutral `core/utils` contract where needed.
Cross-domain data is carried through public provider contracts and translated
by a product-form root composition; a platform never imports a machine snapshot
type. All guest-state mutation occurs on the machine execution thread at a
command boundary.

Forbidden dependencies are any core-to-VM/mantle/dos/VDM path, any VM-to-mantle,
DOS, or VDM path, any mantle-to-DOS or VDM path except its declared provider
contract, all DOS-to-component paths, sibling module includes, profile-to-product
construction, platform-to-guest-state mutation, and all dependency cycles.
The build-target graph follows the same rules: a target may not conceal a
forbidden source edge through an aggregate library.

## Session Readiness

An implemented module is session-ready only when every mutable datum is
session-owned, execution-thread-owned, caller-owned invocation state, or an
explicit process-exclusive host lease. Immutable tables and descriptors may be
shared. A process-global, `_Thread_local`, or implicit-current-object selector
is not an acceptable production-state owner. The current closure authority is
the [M5 closure checklist](../planning/m5-closure-checklist.md).
An undocumented mutable file-static object is never an acceptable substitute
for a session context or host lease.

## M5 Convergence

M5 converges the implemented `core` and `vm` roots, retains only the bounded
non-runnable VDM skeleton, and keeps the root foundation units `type.*`.
`mantle/` and `dos/` remain architecture-only until their respective admission
milestones; they do not enter the current NXVM build graph.

The completed migration rationale is summarized in
[M5 History](../history/m5.md). Current work must meet this document's
ownership and dependency rules directly; completed plans cannot create an
exception. Shared concrete Win32/Linux host providers move to `core/platform`
only when proven mechanism-only; VM policy remains in VM composition, reusable
VDM composition remains in mantle, and NXVDM policy remains in VDM.
