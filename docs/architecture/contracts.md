# Contract Design

## Authority

This is the sole forward interface-design authority. It complements
`module-layout.md`: that document owns source boundaries, while this document
owns the public C contracts crossing those boundaries. Requirements describe
observable behavior; planning documents do not define interfaces.

## Foundation

`src/type.h` is the common system type header. It defines `ntvdm64_status`,
retained NXVM numeric aliases, common bit/constant helpers, and product-neutral
legacy C-runtime and trace primitives. `src/type.c` owns their non-inline
implementations. [C-Library Facade](c-library-facade.md) is the sole authority
for its C vocabulary and header boundary.

Each product module owns its `PRODUCT_NAME`. The shared core-product banner
helper defines `PRODUCT_VERSION`, `PRODUCT_COPYRIGHT`, and
`PRODUCT_BUILD_TIME`; the product entry includes both before printing. No
module contract contains an ABI version, timestamp, compatibility probe, or
module-local status type.

Public symbols use source-path ownership: `core_machine_*`,
`core_platform_*`, `core_product_*`, `vm_machine_*`, `vm_platform_*`,
`vm_product_*`, `vm_profile_*`, `mantle_*`, `dos_*`, and `vdm_*`. Root
composition uses `vm_session_*` and `mantle_session_*` for its concrete
session contracts.

The precise distinction among private machine implementation, exposed
interface, injected provider, and session-owned registry is defined in
[Core Machine Instance Design](core-machine-instance-design.md). This document
continues to define the semantics of those public contracts.

## Common Rules

- Public state is opaque; configuration, commands, snapshots, and callback
  payloads are plain C data structures.
- `*_create` transfers a created handle to its caller; `*_destroy` releases
  only resources owned by that module and accepts `NULL`.
- Inputs are borrowed unless an API explicitly registers or retains them.
  Outputs are copied into caller-provided storage or copied callback payloads.
- `ntvdm64_status` distinguishes invalid arguments, invalid state,
  unsupported capability, no memory, and fault. It never carries product
  policy.
- Callbacks state their thread, synchronization, ownership, and teardown rule.
  They do not re-enter mutable operations on their originating object.

## Product Session Management

`core/product/session` is shared product tooling, not a product composition.
`core_product_session_manager` owns the opaque entry table, numerical IDs,
selection, copied snapshots, and generic `SESSION` grammar. It may retain an
opaque concrete-session handle only to return it to its provider; it never
constructs, mutates, runs, or interprets a VM or mantle session.

The manager has a nonempty-table invariant: after initial creation it always
contains at least one live entry and exactly one selected entry. `close` on the
last entry returns a defined invalid-state result without calling the concrete
provider. Closing the selected entry selects the lowest remaining ID.

Each product-root composition provides a `core_product_session_provider` with
`open`, `describe`, and synchronous `close` callbacks. `open` creates the
concrete session; `describe` copies its generic state; `close` must stop, join,
finalize, and destroy it before returning success. The manager removes an entry
only after successful close. A selected-machine provider remains composition
owned and resolves the selected opaque entry to its product's concrete session.

The shared command facility receives caller-owned tokens and output callback;
it does not depend on an NXVM Console or VDM CLI. Product UI may route the
`SESSION` verb to that facility, but must not cache a selected machine/session
pointer or selected ID. Workers and guest execution paths receive only their
own session and never access a manager. `core/composition/` is not a valid
home for this mechanism. The current contract is defined here; its completed
NXVM implementation sequence is summarized in [M5 History](../history/m5.md).

## Contract Sequence

The following sections are completed in order before a migration changes the
corresponding runtime path:

1. `core/machine`: lifecycle, execution, memory/port/interrupt, provider, and
   read-only state contracts.
2. `core/platform`: host capability providers plus platform frame/event
   contracts, with no machine type dependency.
3. `core/product`: generic command, debug, trace, and registry target
   contracts, with no machine or platform type dependency.
4. Root composition: VM/mantle adapters for machine snapshots, platform events,
   product targets, callback binding, and teardown.
5. Profile override: ROM assets, declarative metadata, and limited firmware
   callbacks against public core contracts.

No implementation detail or old M2/M4 contract becomes current merely because
it remains in `history/`.

## Presentation Boundary

A display bridge has two independent payload contracts. The product machine
module owns its snapshot, which may embed `core_machine_text_snapshot` and carry
machine-private diagnostics. The product platform module owns its frame and
its submit or sink contract; the frame carries host-facing copies only and
must not embed, reference, or name a machine snapshot type.

Only `vm` or `mantle` composition may include both contracts. At its defined
execution boundary it maps a machine snapshot to a platform frame and submits
the frame. Neither `core/platform` nor a product platform module may include a
machine header. A presentation probe injects a platform-sink spy through the
platform contract and verifies copied cells, attributes, geometry, cursor,
generation, and boundary timing; a dependency gate rejects machine-to-platform
or platform-to-machine sibling includes.

## Core Machine: Lifecycle And Cooperative Execution

`core/machine` owns no host thread and exposes no `start` function or internal
infinite loop. It is a deterministic guest execution library: root composition
owns product threads, host event policy, realtime pacing, wall-clock watchdogs,
and process exit.

- `core_machine_create` creates CPU, RAM, bus, and execution state in a
  configuring state. It does not execute guest instructions.
- Composition registers execution and port providers while configuring, then
  calls `core_machine_freeze_execution_providers`. Provider topology, port/IRQ
  ownership, and firmware service registration are immutable after freeze.
- `core_machine_reset` resets execution state and invokes frozen provider reset
  callbacks. A topology change is a root-composition reconstruction, not an
  ordinary reset.
- `core_machine_run` is synchronous and accepts a finite instruction budget.
  It may begin from `STOPPED` after a completed reset or resume from `PAUSED`.
  Core has no host-time budget; a root composition owns wall-clock limits.
- `core_machine_run` returns when its budget is exhausted or earlier for a
  pause, stop request, guest/provider stop, or fault. A normal product loop
  immediately issues the next quantum when its own policy permits.
- `core_machine_request_stop` is the only cross-thread machine-control entry.
  It records a stop request and the execution thread observes it at a defined
  guest boundary.
- `core_machine_destroy` releases only core-machine resources. Root composition
  detaches and destroys providers, product UX, and platform objects.

An instruction budget is an execution quantum, not a debugger-only limit. For
example, VM composition may run 100,000 instructions, drain host events and
publish output, then run another quantum. A debugger step uses the same path
with a budget of one. This prevents an unbounded guest loop from owning the
host control flow while preserving one execution implementation.

VM and mantle composition may share a `core/product` queue, wake, and drain
primitive only after both loops have a demonstrated identical mechanism. Such
a primitive knows no machine or platform type and never decides scheduling,
display policy, cancellation, boot continuation, or program exit. Those remain
VM/mantle root-composition policy.

## Core Machine: Configuration, State, And Run Result

`core_machine_config` contains static core-machine capability: RAM capacity and
the frozen CPU/FPU profile selections. VM/mantle composition translates a
selected profile into this configuration.

It contains no profile identifier, ROM/BIOS/CMOS data, storage device, host
resource, window/Console option, debugger option, or product exit policy.
Those are provider, product, or root-composition concerns.

CPU and FPU selection use core-owned profile enums, not VM or VDM model names.
This lets the core evolve its x86 implementation without importing product
profile semantics.

`core_machine_lifecycle` exposes `INITIALIZED`, `STOPPED`, `RUNNING`,
`PAUSED`, and `FAULTED`. `INITIALIZED` is the configuration window and is not
runnable. A completed reset, including the initial reset before first `START`,
ends in `STOPPED`. `RUNNING` exists only while a synchronous
`core_machine_run` call is active. `PAUSED` is a returned execution boundary
after guest execution has deliberately paused or yielded; it is not an
unstarted or reset machine. `STOPPED` is never a limbo state: its CPU, RAM,
shared devices, and frozen providers have completed reset, its reset vector is
valid, and a subsequent run may cold-start without another initialization
transition. A running stop or reset request performs this cold reset before
`STOPPED` becomes observable; reset failure must not publish a half-initialized
stopped state. `FAULTED` requires an explicit reset.

`core_machine_run_result` reports why one quantum returned:

- `QUANTUM_COMPLETE`: instruction budget exhausted and execution may continue.
- `WAITING_FOR_INTERRUPT`: CPU is halted awaiting an interrupt or another
  execution-boundary event; root composition owns any host wait or wake.
- `PAUSED`: a debugger or explicit pause boundary was reached.
- `STOP_REQUESTED`: root composition requested a safe stop.
- `PROVIDER_STOP`: a registered provider requested termination with provider
  detail; only root composition interprets that detail as product behavior.
- `FAULT`: core reports a machine/CPU fault and guest location detail.

The core does not define a DOS program exit or a whole-PC process exit result.

## Core Machine: CPU State And Physical Memory

`core_machine_get_cpu_state` copies `core_machine_cpu_state`, including the
current code location, flags, and halt state. There is no public CPU-state
setter. CPU state and the first-fault diagnostic are observable only at a
returned paused/stopped/faulted execution boundary; a caller uses one combined
copied observation payload when it needs both values from the same quantum.

`core_machine_memory_read` and `core_machine_memory_write` accept only a
physical address, caller storage, and `size_t` length. The current public
contract uses a `uint32_t` physical address; wider translation helpers require
a future explicit CPU/debug contract. Access is range checked and observes the
current A20 state. Neither function returns a writable raw RAM pointer.

Segment:offset translation, linear addressing, paging translation, and CPU
mode interpretation are core CPU semantics. They are not alternate forms of
the generic physical-memory API. A future explicit CPU debug helper may expose
such a translation when required, without weakening this boundary.

CPU and memory mutation occurs only at an execution boundary. A debugger, DOS
loader, firmware override, or root composition uses these APIs only after the
current quantum has returned; `core/product` receives an adapted debug target,
not a `core_machine` handle.

## Core Machine: Frozen Topology And Mutable Guest State

`core_machine_freeze_execution_providers` closes the machine configuration
window and makes the machine topology immutable: physical-address
routing, port ownership, IRQ ownership, and firmware-service registration may
not be added, removed, or rebound until root composition destroys and rebuilds
the machine. It does not make guest-visible contents immutable.

While a quantum runs, guest CPU instructions, DOS, and firmware may freely
change writable RAM and device state through the frozen map. For example, a
guest may write video memory, update interrupt vectors in writable memory, or
change DMA/device registers. A ROM or other read-only region retains the
semantics supplied by its registered provider. Dynamic CPU address behavior,
including A20, affects translation to the same frozen map; it never changes
which provider owns a mapped range.

External memory, port, and A20 operations require a returned `PAUSED` boundary
or a cold-start-ready `STOPPED` boundary. Root
composition delivers host input and product commands through its own boundary,
then the relevant provider or core API applies them on the machine execution
thread. This preserves deterministic guest state without giving platform or
product modules direct access to the machine.

### Configuration Borrows And Reconfiguration

Composition may borrow mutable executor or shared-device implementation
objects only during the `INITIALIZED` configuration window, immediately to
bind a provider/profile callback. A configuration borrow is neither a public
product capability nor a retained pointer: it cannot be cached in session
state and cannot be used by a product command. A provider registered before
freeze may retain the supplied child reference for its own callback lifetime;
core keeps that child storage stable until provider teardown. The historical
generic executor-borrow names are migration debt and must be replaced by
purpose-named configuration borrows.

Physical RAM capacity is configuration, not mutable guest state. A product
request to change it must stop its session and invoke the explicit
`core_machine_reconfigure_memory` cold-reconfiguration operation. The operation
keeps the same core machine, CPU, shared devices, frozen provider topology, and
product session, replaces only the RAM backing allocation, then performs the
normal core and provider reset sequence. It discards guest RAM and execution
state but does not recreate VM media, platform handles, debugger state, or
session identity. CPU/FPU profiles, ROM/profile selection, and port/IRQ
topology remain frozen and require a new session.

RAM records distinguish installed bytes from backing capacity. Ordinary RAM is
mapped only in `[0, installed_bytes)`; an out-of-range physical or real-mode
access fails through the relevant machine access path and never folds modulo
the installed size. The current core has no alternate ROM or hole mapping
provider, so it has no implicit high-address alias. Future profile-defined
regions require an explicit mapped-memory provider contract. Providers retain a
`t_ram *` only and use checked memory operations; they must not cache or expose
the backing base address.

### Frozen Device-Memory Routing

An admitted core device may own guest-visible device memory without turning it
into a RAM mirror. During `INITIALIZED`, `core/machine/memory` may register one
or more non-overlapping checked device-memory providers for exact physical
ranges, then freezes that routing with the rest of the machine topology. Every
physical read and write, including CPU, debugger, and firmware access, first
uses that frozen route and otherwise retains ordinary RAM semantics. A provider
owns its device storage and may expose only copied read/write results; it never
returns a backing pointer, mutates registration after freeze, or delegates its
state to platform/profile/product code.

T238 uses this contract for VADP-owned planar VRAM. It is a single real device
owner, not a second core machine, synchronised RAM shadow, or host framebuffer.

## Core Machine: Provider Scope

A provider is an optional or configured machine behavior registered through a
core-machine contract. It is not a synonym for every core-machine component.
CPU execution, RAM, and the routing mechanisms for ports, mapped address
ranges, interrupts, and reset are indispensable product-neutral core
mechanics. They remain core implementation, not replaceable providers.

A behavior is a provider when a product form, machine profile, or firmware
policy must be able to select, configure, replace, or omit it without changing
those core mechanics. The core contract provides deterministic registration
and dispatch; a provider implementation belongs to the narrowest owner that
can reuse it:

- A reusable PIC, PIT, DMA, CMOS, or generic video model may be implemented in
  `core/machine` and registered with a profile-selected configuration.
- A full-PC-only FDC or HDC belongs in `vm/machine` and registers through the
  same core port, IRQ, DMA, and reset contracts.
- A DOS service such as an INT 21h handler belongs in `dos/machine`; it is not
  a core or mantle implementation.
- A PC-specific BIOS interrupt handler or ROM behavior belongs to its VM
  profile as a restricted firmware override provider.

For example, VM composition reads a profile, creates a reusable core PIC with
that profile's port and IRQ configuration, then registers it before freeze. A
CPU test can instead register a small fake port or interrupt provider and run
without booting a full PC. The provider boundary therefore keeps core usable
for focused instruction tests and lets VM/DOS profiles differ without teaching
core about PC/AT, DOS, Windows, or a host OS.

A generic block capability is a core-machine provider contract, not an HDC
implementation. VM composition may bind its VM HDC to that capability so a
profile firmware handler can read immutable reset-time geometry and perform
sector operations without including VM machine code or accessing global device
state. Core thereby defines the data/command boundary, while VM retains its
controller and media policy.

## Core Machine: Hardware IRQ

Hardware IRQ delivery and a guest `INT n` instruction are separate mechanisms.
CPU `INT` decoding always uses ordinary guest-IVT transfer; core has no
firmware-interrupt portal, software-interrupt provider, or profile-private
decoder bypass.

`core_machine_pic_irq_source` is the only device-facing hardware IRQ boundary.
A source binds one IRQ during the configuration window and may only assert or
deassert its own physical signal. It owns neither a CPU pointer nor a vector,
IRR, ISR, mask, priority, EOI, or cascade state. The core PIC pair owns all of
those controller-visible states and the CPU consumes the resulting vector only
through its existing PIC execution binding.

ICW1 LTIM defines the source lifecycle: an edge-mode low-to-high transition
latches an IRR bit until acknowledgement; a level-mode asserted source is
presented again after EOI until it deasserts. IRQ8--IRQ15 travel through the
slave PIC and master IRQ2 cascade; device sources may not claim IRQ2 directly.
Multiple sources on a line are counted by the core PIC, so one deassertion
cannot withdraw another source's asserted level.

## Core Machine: Read-Only Observation

Core exposes only product-neutral read-only state: machine lifecycle state,
the last run result and fault detail, copied CPU state, and range-checked
physical-memory reads. It does not expose a universal mutable RAM pointer or a
single whole-product snapshot.

Each provider may expose its own copied read-only view through its public
contract. For example, a reusable core video provider may report text cells,
attributes, geometry, cursor, and generation; a PIC provider may report IRQ
diagnostics. VM-only storage-controller detail and VDM-only DOS diagnostic
state remain in their respective product-form machine modules.

VM or mantle composition selects the views needed by its session, copies and
combines them at an execution boundary, and adapts them to a product or
platform contract. A platform receives only a platform frame and never a
`core_machine` handle, guest-memory pointer, DOS-private state, VM media
controller state, or window policy. This permits debugger inspection and
safe presentation refresh without turning core into a whole-product snapshot
schema.

A core video provider marks its presentation state changed when guest execution
alters a display mode or visible content. The mark is machine state, not a host
call. Composition observes it at the next execution boundary, captures the
provider view, translates it to a frame, and submits it to platform. This
replaces profile firmware directly calling a display implementation while
preserving the same refresh cycle.

## Core Platform: Host-Capability Boundary

`core/platform` defines reusable host-capability, platform-event, and
presentation-frame contracts. It has no `core_machine` dependency and knows no
DOS service, VM profile, CLI, debugger policy, product exit status, or window
ownership decision.

A platform provider may produce copied, normalized host events on a host
thread and may consume copied presentation, audio, or log frames. It may not
mutate guest state. VM or mantle composition is the sole bridge: it accepts
platform events into its product-owned queue, consumes them at a machine
execution boundary, and translates machine/provider views into platform frames
before submission.

Concrete host implementations reused by both products belong in
`core/platform/win32` or `core/platform/linux`; platform-neutral contracts and
helpers live directly in `core/platform`. Full-machine window policy remains
in `vm/platform`. VDM parent-Console protection, cancellation semantics, and
drive/path containment remain in `vdm/platform`. A core platform capability
provides a mechanism, never a product policy or a hidden second composition
layer.

## Core Platform: Capability Granularity

Core platform begins with independent, narrow capabilities rather than one
global host-services object:

- An event source produces copied, normalized keyboard, mouse, window, and
  system events.
- Presentation sinks consume copied display frames, audio blocks, and, where
  needed, diagnostic output.
- Host clock capability belongs in `core/platform`; the policy-free injected
  wait callback used across independent owners belongs in `core/utils`.
  Composition owns pacing, waiting, and watchdog policy; `core/machine` never
  reads host time.

Filesystem, drive visibility, serial/parallel policy, and printing do not
enter `core/platform` merely because they touch the host. VM media attachment
and VDM path containment have distinct product and security meaning, so they
remain in `vm/platform` and `vdm/platform` until both products demonstrate an
identical, policy-free byte-stream capability worth promoting to core.

## Core Platform: Event, Frame, And Teardown Ownership

An event source creates a copied normalized event on its host thread.
Composition's event callback may only enqueue or otherwise record that copied
event for later consumption; it may not mutate a machine or call a provider
that mutates guest state. The source `stop` operation is synchronous: when it
returns successfully, no source callback remains active and no later callback
will occur.

A presentation sink copies each submitted frame, audio block, or diagnostic
payload before its submission call returns. Composition retains ownership of
the source data and may reuse or release it immediately after the call.

Composition owns shutdown order: it stops event sources, closes its ingress
queue, requests and observes a machine stop at an execution boundary, then
detaches and destroys machine providers, platform objects, and product UI.
Window close, Ctrl+C, and host-device loss enter this boundary only as
normalized platform events. VM/VDM composition decides whether each event
pauses, exits, cancels, or produces a product result.

## Core Product: Reusable Product Tooling

`core/product` is a reusable product-tool library, not a third product
composition layer. It may contain pure assembler/disassembler operations,
generic command dispatch, abstract debugger operations, structured trace,
bounded trace storage, and explicit registries.

Generic command and debugger tools act only through abstract targets, such as
read CPU state, read memory, step, continue, or set a breakpoint. Root
composition adapts a real machine or provider to that target. Core product
does not include a machine or platform type and never selects a profile,
creates a session, owns an execution loop, receives host events, or determines
product exit semantics.

An explicit registry is a narrow registration/query utility, not a global
service locator through which a module may discover sibling objects. Trace
events are structured, filterable, and capacity-bounded. Their collection,
clearing, and export remain composition and product-UX policy; an unbounded
raw instruction recorder is not a core-product facility.

NXVM Console commands, NXVDM CLI parsing, display/Console ownership, and
product-specific debug interaction belong in `vm/product` or `vdm/product`.
Assembler/disassembler code whose inputs and outputs are pure data belongs in
core product because it accesses neither a machine, a platform provider, nor a
global session.

## Root Composition: Product Integration

`vm/main.c` is the current thin `nxvm.exe` entry point. `vdm/main.c` is the
future thin `nxvdm.exe` entry point and binds mantle to dos when introduced.
`vm_session` and `mantle_session` are the sole integration owners: they may
include the applicable core contracts and all peer modules of their own
component. VDM selects product policy only; no peer module receives this
privilege.

Composition selects a profile; creates core machine state and product-form
machine, platform, and product providers; translates profile descriptions into
registration configuration; binds abstract debug targets and platform event
queues; and freezes the machine. It owns host threads and its product loop:
driving bounded run quanta, consuming queued events, submitting frames,
handling product commands, and applying product exit policy.

For retained NXVM, composition owns the full-PC provider initialization,
reset, refresh, and final ordering as one explicit sequence. It reproduces that
sequence statement-for-statement before any later simplification; device code
does not infer or alter the order. This is the path for moving lifecycle and
host-start calls out of `vm/machine` without changing boot behavior.

Composition reports factual results; VM or VDM product policy translates them
into observable behavior. VM may return to or pause its retained Console;
NXVDM may produce a guest exit code, a cancellation result, or a CLI failure.
Core, mantle, DOS, and peer providers never make either decision.

Composition shuts down in reverse dependency order: stop platform event
sources, close ingress, request and observe a machine stop at an execution
boundary, then detach and destroy providers, platform objects, product UI, and
core machine state. VM and mantle may have similar loops, but sharing a
mechanism must never import VM boot behavior or NXVDM program-run semantics into core.

## Profile: Immutable Blueprint And Firmware Assets

A profile is an immutable declaration: required machine or DOS capabilities,
device mapping, defaults, ROM assets, and provider metadata. It does not
create a machine, start a thread, choose a product interaction policy, or hold
mutable session state.

A profile owns its ROM and other static assets. Root composition retains the
profile for the complete session; registered providers may borrow its immutable
asset data only while that session and its providers remain alive. Composition
destroys those providers before releasing the profile. It translates profile
metadata into concrete provider configuration and registration, rather than
giving arbitrary providers a global profile object.

A profile-specific firmware override may own private provider context and use
only public `core/machine` service and provider contracts to affect guest
state. It may not access platform, product, composition, host resources, or
peer modules directly. VM profiles describe machine models and firmware;
DOS profiles describe DOS memory, service, and device policy. They share this
discipline but have no universal all-component profile object.

Profiles exclude machine-local paths, CLI arguments, window/Console choices,
and other product-session policy. They are reproducible read-only blueprints;
composition turns a selected blueprint into a running session.

## Cross-Module: Resource, Failure, And Callback Rules

Creation, registration, freeze, and reset failures return a factual
`ntvdm64_status` synchronously. They leave no half-registered or half-frozen
object; callers still destroy every object whose creation succeeded. Runtime
providers may continue normally, request a safe machine stop, or report a
machine fault. They never exit a process, close a window, or interpret a
result as a DOS program exit code.

Only root composition translates a lower-level result into retained NXVM
Console behavior, a VDM CLI result, user-visible diagnostics, or product exit
policy. This preserves the distinction between machine fact and product
meaning.

Callbacks never re-enter mutable operations on their originating object.
Cross-thread callbacks transfer copied data only and are synchronously closed
by the relevant `stop` or `destroy` operation. Unless an API explicitly states
otherwise, creators destroy their objects, inputs are borrowed, and outputs
are copied. Any exception must be visible in both its API name and contract.
