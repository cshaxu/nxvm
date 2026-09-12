# Source Layout Detail Record

> Supporting M1--M5 migration and interface detail. The current macro layout
> authority is [design/CODING.md](../../design/CODING.md). This record cannot
> change current component ownership, directory structure, or dependency rules.

## Authority

This preserves former detailed layout reasoning and migration evidence. Source
moves use `git mv`; copied or independently rewritten NXVM implementations are
prohibited.

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
closure is [Core Machine Public-Surface Inventory](../history/m5/core-machine-public-surface-inventory.md);
it is a task design record, not an additional runtime contract.

## Foundation Units

`src/type.h` is the sole common type header and legacy diagnostic foundation.
It supplies product-neutral status, retained NXVM numeric aliases, common
bit/constant helpers, and shared runtime/trace primitives. `src/type.c` owns
their non-inline implementations. All modules may include `type.h`; this is a
foundation-unit dependency, not a dependency between product forms or modules.
The detailed C vocabulary and header boundary are defined only by
[C-Library Facade](../history/m5/c-library-facade.md).

`src/core/utils/` is the sole shared utility module. It contains only small,
product-neutral facilities and callback contracts that require neither guest
state nor host policy, such as an injected wait scope. It depends only on
`type-facade`; it may not include any `core/{machine,platform,product}` or
`vm/*`/`vdm/*` header. It is not a miscellaneous convenience layer: assembler,
disassembler, debugger UX, registries, device code, platform implementation,
and product policy remain with their named owners. Public symbols use the
`core_utils_*` prefix.

NXVM's root `src/banner.h` supplies its entry banner's version, copyright and
build time. No
module contract contains an ABI version, timestamp, or compatibility probe;
the repository is one synchronously built system.

Public C symbols use their source ownership path: `core_machine_*`,
`core_platform_*`, `vm_machine_*`, `vm_platform_*`,
`vm_product_*`, `vm_profile_*`, `mantle_*`, `dos_*`, and `vdm_*`. Root
composition exports its concrete session as `vm_session_*` or
`mantle_session_*`. Internal composition helpers remain private to their
component root.

Composition implementation and private headers live under
`vm/composition/` or `mantle/composition/`. The component root owns that directory;
it is not a fourth module and does not relax the directed module dependency
rules. Product entry points remain directly under `vm/main.c` and `vdm/main.c`.

`common/session` owns only generic session reduction and copied facts; it never
creates or understands an NXVM machine. VM owns its concrete construction,
provider implementation, selected-item adaptation and teardown. Product UI
receives copied snapshots and never owns or caches a Core machine pointer.
`core/composition/` is forbidden because it would be an accidental second
product assembly layer.

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

`lib` contains product-neutral C and host facilities. `common` contains generic
command, debug, assembler, disassembler, session and UI contracts, but no
session composition, NXVM Console, profile, boot/media or host-policy decision.
Its components depend only on Lib, never on one another. Root composition owns
concrete product binding and lifetime.

The hardware-debugger command language, prompt and pure assembler/disassembler
capability belong in `common/debug` and `common/xasm32`. The common machine
contract declares the copied operations it needs; each product adapter binds
those operations to its own machine without forking the shared grammar.

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

Ownership is determined by reuse, not abstraction level or legacy directory.
Policy-free host and C capabilities belong in `lib`; reusable product capability
belongs in `common`; guest execution belongs in `core/machine`; product-only
implementation belongs in `vm/*`. No legacy `core/utils`, `core/platform` or
`core/product` owner remains.

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
`lib/storage`, `lib/host` and `lib/ui-*` own opaque host files, waits and native
presentation. They return copied values or opaque Lib handles and never
interpret mount, profile or product policy. `vm/product` owns retained NXVM
user experience: Console, debugger integration, media commands and presentation
policy. `vm/profile` owns VM
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
lib                 common
 |                   |
 +---- core/machine -+---- vm/{app,machine,product,profile} -> nxvm.exe
```

`core/machine` is the sole Core owner for mutable guest state and guest-domain
contracts. It depends on neither VM nor Common. `lib` wraps native facilities;
its public surface carries no native handle. `common` depends only on Lib and
contains no Core, VM, profile, media or native-platform type. VM is the one
product composition point that binds Core to Common and Lib.

Within VM, `machine`, `product`, and `profile` retain distinct owners. A profile
is declarative data and provider metadata, not a machine constructor. Adapters
which translate input, display snapshots or callbacks belong to VM, rather than
creating a cross-owner dependency. VM owns host threads, pacing and product
exit policy.

Cross-domain data is carried through public provider contracts and translated
by the product root. All guest-state mutation occurs on the machine execution
thread at a command boundary.

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
the [M5 closure checklist](../operations/policy/m5-closure-checklist.md).
An undocumented mutable file-static object is never an acceptable substitute
for a session context or host lease.

## M5 Convergence

M5 converges the implemented `core` and `vm` roots, retains only the bounded
non-runnable VDM skeleton, and keeps the root foundation units `type.*`.
`mantle/` and `dos/` remain architecture-only until their respective admission
milestones; they do not enter the current NXVM build graph.

The completed migration rationale is summarized in
[M5 History](../history/legacy/m5.md). Current work must meet the current
source-layout authority, not this record's historical phrasing.
ownership and dependency rules directly; completed plans cannot create an
exception. Shared concrete Win32/Linux host providers move to `core/platform`
only when proven mechanism-only; VM policy remains in VM composition, reusable
VDM composition remains in mantle, and NXVDM policy remains in VDM.
