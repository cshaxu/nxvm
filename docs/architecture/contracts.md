# Contract Design

## Authority

This is the sole forward interface-design authority. It complements
`module-layout.md`: that document owns source boundaries, while this document
owns the public C contracts crossing those boundaries. Requirements describe
observable behavior; planning documents do not define interfaces.

## Foundation

`src/type.h` is the common system type header. It defines `BOOL`, `SIZE_T`,
fixed-width aliases, `STATUS`, and the stable `STATUS_*` result constants.
`src/type.c` exists only if a non-inline type helper is necessary.

`src/version.h` and `src/version.c` are the only version and build-identity
source. They provide product banner identity and build-time information. No
module contract contains an ABI version, timestamp, compatibility probe, or
module-local status type.

Public symbols use source-path ownership: `core_machine_*`,
`core_platform_*`, `core_product_*`, `vm_machine_*`, `vm_platform_*`,
`vm_product_*`, `vm_profile_*`, and VDM counterparts. Root composition uses
`vm_composition_*` and `vdm_composition_*`.

## Common Rules

- Public state is opaque; configuration, commands, snapshots, and callback
  payloads are plain C data structures.
- `*_create` transfers a created handle to its caller; `*_destroy` releases
  only resources owned by that module and accepts `NULL`.
- Inputs are borrowed unless an API explicitly registers or retains them.
  Outputs are copied into caller-provided storage or copied callback payloads.
- `STATUS` distinguishes invalid arguments, invalid state, unsupported
  capability, no memory, busy, and fault. It never carries product policy.
- Callbacks state their thread, synchronization, ownership, and teardown rule.
  They do not re-enter mutable operations on their originating object.

## Contract Sequence

The following sections are completed in order before a migration changes the
corresponding runtime path:

1. `core/machine`: lifecycle, execution, memory/port/interrupt, provider, and
   read-only state contracts.
2. `core/platform`: host capability providers plus platform frame/event
   contracts, with no machine type dependency.
3. `core/product`: generic command, debug, trace, and registry target
   contracts, with no machine or platform type dependency.
4. Root composition: VM/VDM adapters for machine snapshots, platform events,
   product targets, callback binding, and teardown.
5. Profile override: ROM assets, declarative metadata, and limited firmware
   callbacks against public core contracts.

No implementation detail or old M2/M4 contract becomes current merely because
it remains in `history/`.

## Core Machine: Lifecycle And Cooperative Execution

`core/machine` owns no host thread and exposes no `start` function or internal
infinite loop. It is a deterministic guest execution library: root composition
owns product threads, host event policy, realtime pacing, wall-clock watchdogs,
and process exit.

- `core_machine_create` creates CPU, RAM, bus, and execution state in a
  configuring state. It does not execute guest instructions.
- Composition registers core-machine providers while configuring, then calls
  `core_machine_freeze`. Provider topology, port/IRQ ownership, and firmware
  service registration are immutable after freeze.
- `core_machine_reset` resets execution state and invokes frozen provider reset
  callbacks. A topology change is a root-composition reconstruction, not an
  ordinary reset.
- `core_machine_run` is synchronous and accepts a finite instruction budget.
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

VM and VDM root compositions may share a `core/product` queue, wake, and drain
primitive only after both loops have a demonstrated identical mechanism. Such
a primitive knows no machine or platform type and never decides scheduling,
display policy, cancellation, boot continuation, or program exit. Those remain
VM/VDM root-composition policy.

## Core Machine: Configuration, State, And Run Result

`CORE_MACHINE_CONFIG` contains only static core-machine capability: RAM
capacity, CPU architecture capability bits, address-space limits, and baseline
machine settings such as A20 reset policy. VM/VDM root composition translates a
selected profile into this configuration.

It contains no profile identifier, ROM/BIOS/CMOS data, storage device, host
resource, window/Console option, debugger option, or product exit policy.
Those are provider, product, or root-composition concerns.

CPU selection is expressed as core capability bits, not VM or VDM model names.
For example, a VM profile may require 386 instruction capability, while root
composition supplies the corresponding core bit set. This lets core evolve its
x86 implementation without importing product profile semantics.

`CORE_MACHINE_STATE` exposes only `CONFIGURING`, `READY`, `RUNNING`, `PAUSED`,
`STOPPED`, and `FAULTED`. `RUNNING` exists only while a synchronous
`core_machine_run` call is active; `READY` means another quantum may begin.

`CORE_MACHINE_RUN_RESULT` reports why one quantum returned:

- `QUANTUM_COMPLETE`: instruction budget exhausted and execution may continue.
- `PAUSED`: a debugger or explicit pause boundary was reached.
- `STOP_REQUESTED`: root composition requested a safe stop.
- `PROVIDER_STOP`: a registered provider requested termination with provider
  detail; only root composition interprets that detail as product behavior.
- `FAULT`: core reports a machine/CPU fault and guest location detail.

The core does not define a DOS program exit or a whole-PC process exit result.

## Core Machine: CPU State And Physical Memory

`core_machine_get_cpu_state` copies `CORE_MACHINE_CPU_STATE`, including general
registers, segments, flags, instruction location, execution mode, and fault
location. `core_machine_set_cpu_state` replaces that state only while the
machine is `CONFIGURING`, `READY`, or `PAUSED`; it is forbidden while
`RUNNING`. The contract exposes no separate register-setter API.

`core_machine_mem_read` and `core_machine_mem_write` accept only a
`CORE_MACHINE_ADDRESS` physical address, caller storage, and `SIZE_T` length.
`CORE_MACHINE_ADDRESS` is represented by `U64` even where a current x86
profile uses only a smaller range. Access is range checked and observes the
current A20 state. Neither function returns a writable raw RAM pointer.

Segment:offset translation, linear addressing, paging translation, and CPU
mode interpretation are core CPU semantics. They are not alternate forms of
the generic physical-memory API. A future explicit CPU debug helper may expose
such a translation when required, without weakening this boundary.

CPU and memory mutation occurs only at an execution boundary. A debugger, DOS
loader, firmware override, or root composition uses these APIs only after the
current quantum has returned; `core/product` receives an adapted debug target,
not a `CORE_MACHINE` handle.

## Core Machine: Frozen Topology And Mutable Guest State

`core_machine_freeze` makes the machine topology immutable: physical-address
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

External mutations remain serialized at an execution boundary. Root
composition delivers host input and product commands through its own boundary,
then the relevant provider or core API applies them on the machine execution
thread. This preserves deterministic guest state without giving platform or
product modules direct access to the machine.

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
- A DOS service such as an INT 21h handler belongs in `vdm/machine`; it is not
  a core DOS implementation.
- A PC-specific BIOS interrupt handler or ROM behavior belongs to its VM
  profile as a restricted firmware override provider.

For example, VM composition reads a profile, creates a reusable core PIC with
that profile's port and IRQ configuration, then registers it before freeze. A
CPU test can instead register a small fake port or interrupt provider and run
without booting a full PC. The provider boundary therefore keeps core usable
for focused instruction tests and lets VM/VDM profiles differ without teaching
core about PC/AT, DOS, Windows, or a host OS.

## Core Machine: Hardware IRQ And Firmware Service

Hardware IRQ delivery and a guest `INT n` instruction are separate mechanisms.
Device providers such as PIC, PIT, or keyboard use the core hardware-IRQ
contract to request CPU interrupt delivery. A guest `INT n` instruction remains
normal CPU semantics: unless a registered firmware service handles it, core
uses the guest interrupt vector table and transfers to guest code.

A firmware service is an optional software-interrupt provider. Before freeze,
at most one service may register for one interrupt vector; duplicate claims
are a configuration error. Core dispatches the registered service first, and
its disposition is exactly one of:

- `HANDLED`: the service completed the interrupt and core resumes guest
  execution according to the service-return contract.
- `PASS_TO_GUEST`: core performs the ordinary guest-IVT transfer for that
  `INT n`; it does not try another firmware provider.
- `FAULT`: core ends the quantum with defined guest-fault detail.

For example, a PC110 profile firmware override may handle only its private
`INT 15h` function and return `PASS_TO_GUEST` for all other functions, which
then execute the ROM BIOS entry recorded in the IVT. A VM that uses only a ROM
BIOS registers no service for `INT 10h`, so the instruction always follows the
ordinary IVT path. In VDM, the owned DOS `INT 21h` service is the one handler;
unsupported DOS functions receive defined DOS error semantics from that
service and still return `HANDLED`, rather than falling through to an arbitrary
guest vector.

Profiles provide service metadata and profile-specific override code, but they
do not set precedence or invent a hidden service chain. When several HLE
handlers must cooperate, VM or VDM root composition explicitly constructs one
composite service, defines and tests its internal order, and registers that
single service with core. This keeps dispatch order, fallback, and fault
meaning uniform across products.

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

VM or VDM root composition selects the views needed by its product, copies and
combines them at an execution boundary, and adapts them to a product or
platform contract. A platform receives only a platform frame and never a
`CORE_MACHINE` handle, guest-memory pointer, DOS-private state, VM media
controller state, or window policy. This permits debugger inspection and
safe presentation refresh without turning core into a whole-product snapshot
schema.

## Core Platform: Host-Capability Boundary

`core/platform` defines reusable host-capability, platform-event, and
presentation-frame contracts. It has no `CORE_MACHINE` dependency and knows no
DOS service, VM profile, CLI, debugger policy, product exit status, or window
ownership decision.

A platform provider may produce copied, normalized host events on a host
thread and may consume copied presentation, audio, or log frames. It may not
mutate guest state. VM or VDM root composition is the sole bridge: it accepts
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
- Clock and wake primitives support composition-owned pacing, waiting, and
  watchdogs; `core/machine` never reads host time.

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
