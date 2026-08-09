# Machine Contract Detail Record

> Supporting M1--M5 interface detail. The current high-level architecture is
> [design/ARCHITECTURE.md](../../design/ARCHITECTURE.md); this record preserves
> technical contract rationale and cannot establish a competing current rule.

## Authority

This record preserves detailed public-C contract decisions made during M1--M5.
Current source boundaries are owned by [design/CODING.md](../../design/CODING.md),
and active implementation work must state any newly admitted interface in its
task record and history.

## Foundation

`src/type.h` is the common system type header. It defines `ntvdm64_status`,
retained NXVM numeric aliases, common bit/constant helpers, and product-neutral
legacy C-runtime and trace primitives. `src/type.c` owns their non-inline
implementations. [rules/CODING.md](../../rules/CODING.md) defines the current C
vocabulary and header boundary; [C-Library Facade Detail Record](c-library-facade.md)
preserves the M5 inventory and rationale.

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
home for this mechanism. This record preserves the M5 contract detail; its
completed NXVM implementation sequence is summarized in
[M5 History](../legacy-history/m5.md).

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

For text payloads, cursor coordinates are copied facts with
`cursor_x=column` and `cursor_y=row`, relative to the CRTC display-start
address. VADP derives visibility from the CRTC cursor address and inclusive
scanline shape; an off-page or disabled cursor is not wrapped into the visible
frame. Platform consumers may only render those copied facts and cannot infer
or modify guest VRAM, CRTC state, or raster phase.

### EGA Mode 10h Direct Contract (T284/T285)

T284 froze the contract that T285 implements for the first Windows-facing
graphics target: BIOS `INT 10h` set mode `10h`, EGA `640x350x16` planar direct.
T285 adds only this bounded mode and retains the existing Console/debugger UX
and text, CGA, and EGA `0Dh` behavior.

The state owner remains `core/machine` VADP. VADP is the only owner of video
registers, CRTC-derived display geometry, four-plane EGA VRAM, latches, dirty
generation, and copied display snapshots. The default PC/AT profile may bind
only ports, address windows, initial device configuration, and ROM bytes. The
default ROM may request mode changes only by guest-visible port/memory
transactions. Platform code consumes copied snapshots only; it never borrows
guest VRAM, interprets EGA registers, or owns a renderer-side video state
shadow.

The implemented T285 contract is:

- keep the A0000h `64 KiB` per-plane aperture and existing planar/latch read
  and write semantics;
- select mode 10h only when the ROM's guest-visible CRTC `offset` is `40`
  words: this fixes `640x350`, `16` indexed colors, `80` bytes per scanline
  per plane, and plane-to-pixel composition from bits 0--3;
- admit only the CRTC fields needed for geometry, stride, display-start, and
  cursor/text restoration; unsupported CRTC fields remain deferred;
- expose a copied indexed frame that can hold `640 * 350` pixels and the
  fixed 16-entry EGA RGBI palette selected through the existing attribute
  palette registers;
- provide the ROM boundary for `INT 10h AH=00h AL=10h` and the required return to
  mode `03h`, with BDA mode/query state consistent with the retained INT 10h
  query service.

This contract does not admit VGA, DAC programming, VBE, 256-color modes, VGA
font/text-plane behavior, composite video, light pen, arbitrary EGA modes, a
CRTC rewrite, a VM-side VRAM shadow, a host shortcut, or a profile callback
that bypasses VADP. Existing `EGA-320x200x16-direct` behavior remains frozen:
its planar aperture, latch semantics, `320x200` copied frame, RGBI palette,
ROM `0Dh` selection, and mode `03h` restoration must not change merely because
mode `10h` is designed.

T285 turns the T284 corpus into normal success evidence. The core corpus
checks mode kind, `640x350` capacity, CRTC offset and nonzero display start,
four-plane composition, palette selection, and the `y=349` tail row. The VM
corpus boots an owner-built floppy, requests mode `10h`, writes all four
planes, captures the copied frame, verifies BDA/query state, and returns to
text mode. No Windows media is committed; a lawful user-supplied Windows
fixture remains extra observation evidence under a later admission.

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

### Bounded 80386 CPL0 Paging

When a frozen 80386 profile has enabled both PE and PG, the one core executor
uses `logical -> segment cache -> linear -> 4 KiB PDE/PTE walk -> physical
memory route`. Page-table reads and Accessed/Dirty updates use the same checked
core physical-memory route as ordinary guest accesses; no VM, firmware, host
MMU, or raw-RAM path participates. T258 admits only `MOV r32,CR0`,
`MOV r32,CR2`, `MOV CR0,r32`, and `MOV CR3,r32`: CR2 is fault-written only,
CR0 changes only PE/PG and cannot set PG before PE, and CR3 must be 4 KiB
aligned. Other guest control-register writes stop with `#UD`.

The paging path itself remains ring 0 only. Non-present fetch/read/write faults
retain their original instruction point, `CR2`, and P/W/U diagnostic bits as a
copied core `FAULT` result. T259 separately admits a 16-bit CPL3 software-INT
gate and `#GP` delivery subset. T260 additionally admits the 80386
32-bit-TSS I/O-map decision for the retained CPL3 path: every byte of an
IN/OUT span must be present and clear before the port provider is reached;
denial is `#GP(0)`. T261 separately admits CPL0 far `JMP` from one GDT 16-bit
TSS to another: core validates both state images before save/restore, clears
the old busy bit, sets the new busy bit, materializes SS as a stack cache while
DS/ES remain data caches, and retains the original `#GP`, `#NP`, or `#TS`
diagnostic on pre-commit failure. This does not prove CPL3
paging permission, CR0.WP write protection, TLB behavior, task switching
under paging, 32-bit TSS switching, task gates, far CALL, nested task return,
or host-assisted recovery.

CPU and memory mutation occurs only at an execution boundary. A debugger, DOS
loader, firmware override, or root composition uses these APIs only after the
current quantum has returned; `core/product` receives an adapted debug target,
not a `core_machine` handle.

### Core Configuration And Provider Boundary

`core_machine` owns CPU, RAM, port bus, shared controller storage, and their
initialization, reset, and finalization order. During the one configuration-open
window, a product composition may submit only typed configuration and provider
bindings. It may not retain or receive a raw CPU, RAM, port, controller, or
executor pointer. After core validates and freezes those bindings, composition
owns outer scheduling and provider lifetime while `core_machine_run()` remains
the only guest execution loop.

An override firmware provider is core-invoked only at an explicit machine
service boundary. Before implementation, its contract freezes an operation
enum plus lifecycle, failure/atomicity, re-entry, and nested-call rules. Only
the approved enum may expose checked guest-memory and port operations or
separately admitted copied CPU-state observation or patching. It never exposes
storage pointers, arbitrary instruction-pointer, segment, control-register,
mode, device, or scheduler mutation. A profile declares metadata and provider
inputs, not a machine constructor.

M5 must migrate the current VM construction path to this boundary before M6
creates a mantle runtime. No pre-decode transition registry is part of the
admitted M5 work. Until the applicable implementation work closes, historical
raw configuration/profile borrows are transitional implementation detail, not
a new consumer contract or mantle API.

### Core Port Ownership And Dispatch

`core_machine` owns one frozen port-binding registry. Every core device and
typed composition provider registers its read and write bindings in that
registry during configuration; the registry is also the only dispatch path for
guest CPU `IN`/`OUT` and public `core_machine_bus_read/write`. A port's read
and write directions are independent, but each `(port, direction)` has exactly
one binding. A second registration returns `TYPE_STATUS_INVALID_STATE` without
changing the retained binding or its owner.

Core controller configuration validates that all required directions are free
before it binds FDC, HDC, or RTC ports. Reset clears guest-visible port data but
retains the frozen topology; only destruction releases bindings. Public bus
operations remain stopped/paused boundary operations, while guest I/O is part
of normal execution and reaches the same binding. VDM minimal does not install
substitute PIC, PIT, or KBC handlers: host input enters through the existing
core keyboard submission API and its port observations remain core-owned.

S1 deliberately preserves the legacy callback execution result for a bound
typed provider. Exact propagation of a provider's `type_status` to public bus,
guest execution, trace, and output state is a separate S2 contract; this rule
does not create a fallback or a second port path.

T300 S2 makes that propagation explicit for typed providers: public bus read
and write return the provider's exact non-OK `type_status`; a failed read does
not modify the caller output, and a failed write restores the port data latch.
Both failures produce their ordinary port trace event with `detail` equal to
the returned status, rather than a success detail. Guest `IN`/`OUT` maps any
such failure to the existing internal `#CE(port)` diagnostic. It introduces no
new guest exception, provider fallback, or CPU-visible host status encoding.
Firmware and debugger port capabilities use this same executor-port dispatch:
they preserve a failed read's caller output and restore their temporary write
latch before returning the exact provider status.

### VM Session Construction Atomicity

VM composition publishes a `vm_session` only after its core, copied display
provider, media registry, profile firmware, controller topology, frozen core
providers, control state, and platform request/input transports have all
initialized. Each construction stage returns its first exact `type_status` to
the session factory. On failure, the session is never active or returned to a
caller: composition tears down in reverse ownership order, stopping/discarding
platform transport before provider/control finalization and storage/core
destruction. A later valid creation is independent of the failed attempt.

The default PC/AT profile remains declarative input. Invalid firmware-service
metadata and controller topology are rejected as `TYPE_STATUS_INVALID_ARGUMENT`
rather than becoming a partially bound VM. This contract neither changes core
machine lifecycle nor introduces a host recovery path; Console/window,
debugger, and FDD/HDD boot observe only fully constructed sessions.

### Public Machine And Debugger Interfaces

Public core-machine interfaces include only other public contracts and stable
copyable values. `controller_interface.h` carries a core-issued, opaque DMA
binding nonce and FDC/HDC topology values that composition genuinely submits.
The nonce is not an address or a dereferenceable capability: the receiving
machine validates it against its own private DMA state, so a binding from one
machine is rejected by another. It carries no controller storage or lifecycle
entry. `display_interface.h` carries copied
display capability configuration. Private CPU, decoder, RAM, port, controller,
and VADP layouts remain implementation-only.

`core_machine_debug_instruction_observation` is a copied debugger record, not
a `t_cpu` or `t_cpuins` copy. It names only the registers, segment bases,
instruction location/bytes, code-default-size, and bounded memory accesses
used by the retained VM breakpoint, recorder, and disassembly paths. The core
captures it only at the existing debugger boundary. A source-shape recurrence
gate rejects private core-machine includes or complete private layouts in any
`*_interface.h`, alongside the existing raw-borrow and test-support checks.

## Time And Clock Ownership

Time is not one device category. Each layer owns only its own observable
meaning:

| Concern | Owner | Boundary |
| --- | --- | --- |
| Deterministic `elapsed_ticks`, run budget, and core scheduler order | `core/machine` | `core_machine_run` advances only guest time; it never reads host time. |
| Generic PIT counter/GATE/OUT behavior and its IRQ0 source | `core/machine` | A profile binds the output route; core does not update BIOS data or calendar state. |
| MC146818-compatible register/calendar/event mechanism | `core/machine/rtc.*` | It consumes a frozen tick ratio and emits one configured IRQ source; PC/AT defaults, NMI policy, BIOS, and host-time policy remain outside core. |
| PC/AT CMOS defaults, `70h/71h`/NMI glue, IRQ8 route, BIOS/POST and host-time policy | VM profile/composition | It selects and wires the optional core device without turning those choices into core defaults. |
| BIOS `INT 08h`, BDA tick, and `INT 1Ah` services | VM profile firmware | Firmware consumes normal CPU/PIC delivery; neither core nor platform writes BDA time. |
| Host clock, sleep, pacing, watchdog, and window cadence | root composition/platform | Host time may bound or pace a product loop but never advances guest time directly. |
| DOS date/time APIs | `dos` | DOS maps its own service semantics onto admitted bindings; it does not own a second machine clock. |

At configuration time, VM composition binds a frozen `core_machine_clock_plan`
from its selected profile and registers its execution provider. After freeze,
`core_machine` alone converts its monotonic `elapsed_ticks` into each rational
domain and dispatches DMA, PIT, VADP, KBC, the execution provider, then PIC
visibility in that fixed order. VM composition may pace the host loop and
consume factual run results, but it does not advance VM-owned devices, alter
`elapsed_ticks`, or create a second scheduler. Mantle follows the same rule for
a future runtime; it may bind an admitted optional core RTC but does not
acquire PC/AT defaults, NMI policy, or BIOS behavior.

### Level 2 Instruction Costs

Level 2 refines the input to that same scheduler; it does not create a second
clock. A frozen core-machine timing table supplies a base cost and bounded
additive surcharges for admitted instruction observations. `core_machine_run()`
is the only component that attributes and commits a completed instruction cost,
then advances `elapsed_ticks` and the existing rational domains once. VM
profiles declare the table during configuration; composition, firmware, and
platform cannot add guest ticks.

The initial admitted observations are prefix bytes, a taken short conditional
branch, one selected data-memory MOV form, one IN/OUT operation, and REP MOVSB
iteration count. Instruction fetch, unadmitted forms, bus wait states, DMA
arbitration, prefetch, and host calibration remain outside this contract. The
current executor advances REP once per refresh, so each admitted REP MOVSB
iteration uses `base + rep_iteration_surcharge`; it does not silently obtain a
generic per-memory-access model. A faulted,
stopped, paused, or reset instruction commits no new timing cost. Before an
instruction begins, the executor uses its bounded maximum cost for tick-budget
admission; this may pause conservatively, but never creates different guest
event ordering. Cost or elapsed-tick overflow stops deterministically rather
than wrapping. The legacy `ticks_per_instruction` configuration member remains
only as the zero-surcharge base-cost shorthand for existing consumers.

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

- A reusable PIC, PIT, DMA, or generic video model may be implemented in
  `core/machine` and registered with a profile-selected configuration.
- T273, T276, and T278 moved the optional MC146818, FDC, and ATA controller
  mechanisms into `core/machine` with neutral, explicit configuration. VM
  retains PC/AT wiring/defaults, firmware, media policy, backing objects, and
  UI.
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

### Frozen Core Media Contract (T270)

T270 replaces the retained transitional single-slot block boundary with a
frozen multi-device `core/machine` media-provider contract. A media provider
has a stable opaque device identity; copied capability/status view
(`present`, `removable`, `read-only`, `flushable`, `geometry-known`, and
`change-detectable`); a monotonic generation at the exact mount, eject, or
media-change boundary; copied byte-range and logical-sector I/O; optional
CHS/LBA geometry; format; flush; and typed `absent`, `changed`, `read-only`,
`invalid-range`, `transient`, and `permanent` failures. All byte counts,
alignment, sector sizes, ranges, and generation observations are explicit.

The provider owns its backing state and remains alive through the frozen
machine configuration. Controllers may own a real transfer buffer, but never
a private media cache, geometry copy, or fallback path that compensates for a
contract omission. The provider is neither a controller nor a host-filesystem
interface: it knows no path, handle, mount UI, drive letter, DOS namespace,
persistence policy, or PC/AT topology. The transitional one-slot block API is
retained only until T272 removes its consumers; it must not become a forwarding
facade.

The public surface is `core_machine_media_registry_*` plus copied `query`,
byte-range, logical-sector, format, and flush operations. Registry binding is
permitted only before freeze; all query and I/O operations require the frozen
registry and return a core lifecycle status separately from the typed media
result. The registry owns no provider backing bytes and never returns a
provider-owned pointer.

T271 admits only the distinct `core/platform` opaque backing resource needed
for the media adapter: copied byte-range read/write, size, flush, close, typed
result, and ownership. Existing copied input, wait, and cancellation contracts
are reused rather than duplicated. Composition selects a host path or other
product policy before constructing that opaque resource, then adapts it into a
device-level media provider. Paths, mounts, persistence, drive letters, DOS
namespace, wildcard semantics, and sandbox policy remain above core.

### Opaque Core Platform Backing Resource (T271)

The T271 backing resource is a synchronous, composition-created provider with
copied byte-range `read`/`write`, copied `size`, `flush`, and one synchronous
`close`. It returns a typed resource outcome independently of the core API
lifecycle status: short transfer, read-only, invalid range, transient failure,
permanent failure, unsupported operation, and closed state are explicit.
`close` is the unique transition to closed and guarantees no later provider
call; resource state is then released by its creator. Calls return copied bytes
only, never a native handle or provider-owned buffer.

The resource has no path, directory, stream grammar, mount/eject, sharing,
drive, sandbox, or DOS policy. It creates no thread and invokes no host
callback. Composition owns cancellation and may reuse the existing copied
input/cancellable-wait facilities around bounded resource work; resource I/O
does not create a second cancellation protocol or mutate guest state.

T282 audits and normalizes only existing host-surface contracts with real NXVM
consumers: copied input, wait/cancellation, and presentation/host-surface
leasing. It also records the T271 backing resource as a neutral facility that
does not yet have a production VM consumer. `file`, `directory`, `stream`, and
sampled-host-clock APIs are not admitted merely for symmetry; each needs a real
consumer before a later task may define it. The audit must either make the
public host-surface context opaque to core, or relocate that VM-specific
surface below `vm/platform`; core may manage a lease but never interpret or
operate a native handle.

### VM Media Adapter Boundary (T272)

T272 keeps the VM FDD and HDD backing objects as the sole owners of their
media bytes, geometry, read-only state, insertion state, and generation. Each
implements the frozen core media-provider contract; composition binds those
providers under stable floppy/HDD identities and supplies an already-selected
opaque backing resource only at mount/eject/persistence boundaries. Firmware
may query copied geometry through the media registry but cannot read media
bytes. ATA/HDC retains its direct backing-object use until T277. The FDC is
rebound to the registry in T275 and moves as a neutral controller in T276;
neither transition creates a second media route.

The legacy single-slot block API is removed in T272 after the ROM geometry
consumer is rebound. It must not survive as an adapter, and no controller or
firmware may cache media geometry to bridge its removal.

### Atomic VM Media Replacement And Raw HDD Capacity (T280)

T280 keeps FDD and HDD backing ownership in VM but makes replacement atomic:
inspect, validate, allocate, and completely populate a candidate, then publish
exactly one new in-memory backing state and release the retired byte buffer.
The current backing object has no separately closeable old-media resource. Any
candidate allocation or host-read/close failure discards the candidate and
leaves the old bytes, geometry, read-only state, cursor, presence, and
generation published; it must not leave a half-commit. Explicit persistence
(`remove/save`) writes and closes the requested output before it changes any
published media field, so write/close failure retains the mounted medium.
Persistence creates one uniquely named, exclusively created temporary sibling;
it never truncates or removes an existing temporary candidate it did not create.
FDD
retains its admitted fixed geometry. HDD accepts any **non-sector-aligned raw
length within the admitted in-memory and LBA28 capacity**; its guest-visible virtual capacity is
`ceil(raw_bytes / 512) * 512`, tail reads zero-fill, and a write into virtual
padding makes the next successful persistence output a complete final sector.
That complete length then becomes the backing truth. ATA range checks use the
virtual capacity, bounded only by the admitted LBA28 sector limit and available
candidate allocation; CHS is only a compatibility mapping, never HDD admission
policy. File-backed replacement uses the C facade's 64-bit seek/tell route, so
Windows `long` width cannot truncate a valid raw-image length before admission.
Large or sparse file-backed media needs a separate provider/lifecycle admission;
it is not supplied by this resident-image implementation.

### Existing Host-Surface Boundary (T282)

`core/platform` retains only policy-free copied input, copied presentation, and
cancellable host-wait contracts. A native window, console, or terminal handle
is VM platform policy: its kind, handle storage, and exclusive lease belong in
`vm/platform`, where platform teardown owns release. Core never interprets or
stores a native handle. The generic backing-resource facility has no production
VM consumer and remains a neutral deferred facility; this decision admits no
file, directory, stream, sampled-clock, DOS-path, mount, or guest-time API.

### Neutral RTC Boundary (T273)

`core/machine` owns one per-machine MC146818-compatible device: register
state, date/calendar encoding, elapsed-tick advancement, periodic/alarm/update
flags, selected-register/data access, and its PIC IRQ source. Its configuration
contains only an IRQ binding and frozen tick ratio. A profile may populate
neutral NVRAM bytes during composition, but core contains no PC/AT default,
port selection, NMI policy, BDA/BIOS service, host wall clock, or firmware
source.

VM composition selects the PC/AT ports/IRQ/tick ratio and owns the tiny
70h/71h port provider. Its 70h write extracts bit 7 into the existing VM NMI
policy, then delegates only the lower seven-bit index; 71h delegates data
access to core. It does not copy RTC register, calendar, flag, or IRQ state.
Default-ROM POST and INT assembly are VM profile firmware, never part of the
core controller.

### Neutral FDC Boundary (T276)

`core/machine` owns one per-machine 8272A-compatible controller: command and
result phases, DOR/CCR/MSR/DIR state, transfer cursor, media-generation
observation, DMA request lifecycle, and IRQ6 source lifecycle. Its frozen
configuration is limited to a frozen four-slot drive-to-media identity table,
a DMA request binding, a PIC source route, and explicit port addresses. A
guest command unit and the DOR-selected drive must agree; readiness and media
I/O then resolve only through that selected frozen slot. An invalid or absent
slot produces the established no-data result and cannot fall through to a
different drive. It has no PC/AT defaults, drive-image path, firmware, host
I/O, or product policy.

Each slot retains its own observed media generation and disk-change latch.
Refresh observes every frozen slot, while DIR reports the DOR-selected slot.
Seek/recalibrate acknowledges that selected slot's current generation. A DOR
write that makes an active transfer unready cancels its DMA request and returns
the controller to command phase; a later valid command may request DMA again.
FDC reset retains the established IRQ6 and DMA cancellation route. This is not
rotation timing, a host-clock delay, or a media-topology mutation.

VM composition selects the default-profile FDC port range, IRQ/DMA route, and
drive-0 FDD media identity while the machine is configurable, then supplies
that topology to the one core controller before freeze. The remaining default
drive slots are invalid; a second-drive UI or runtime topology mutation is not
admitted. Default-ROM POST and INT 0Eh/40h assembly remain profile firmware.
No VM object mirrors controller state or accesses FDC media bytes through a
side channel.

### ATA Media and Controller Boundary (T277--T278)

T277 removed ATA PIO's direct `t_hdd` dependency. `core/machine` now owns the
one ATA PIO controller state, primary-master task-file/port protocol, IRQ14
source lifecycle, PIO command/result sequencing, and frozen connection to the
core media registry and selected HDD identity. Present/read-only/geometry
observation and copied sector read/write use the T270 contract.

The controller derives CHS capacity and IDENTIFY words from copied media
geometry, and maps provider failure into its retained ATA abort or ID-not-found
result path. It never receives a backing pointer, image path, host handle, or
media policy. Composition alone selects the HDD provider and binds the PC/AT
port/IRQ/topology declaration before freeze. T277--T278 do not add ATA DMA,
commands, timing, a BIOS shortcut, or a second media route.

### Neutral ATA PIO Boundary (T278)

T278 moved the neutral ATA PIO task-file, PIO buffer, command phases, SRST,
and IRQ source into one `core_machine`-owned controller. Its configuration is
only frozen media registry/id, PIC route, and explicit port/feature fields.
VM composition reads primary-master topology from the profile and binds the
core object during the initialized window; profile firmware retains INT 13h
and boot policy. Core contains no PC/AT default, image path, host I/O,
firmware, or product state. The migration preserves one PIO route and does
not admit ATA DMA, new commands, or another controller.

### ATA nIEN IRQ Visibility (T286)

T286 admits one device-control behavior only: primary-master ATA `nIEN` (bit
1 at `3F6h`) suppresses the controller's PIC-visible IRQ14 assertion for both
an already-pending interrupt and later command completion. It does not alter
the command's PIO data/status phase; clearing `nIEN` restores normal IRQ14
delivery for subsequent completions. The core HDC remains the sole owner of
the bit, pending state, and PIC source lifecycle. This adds no IDE DMA, timing
model, command, reset extension, or VM-side controller mirror.

### Core Controller Media-I/O Evidence (T283)

T283 closes the remaining evidence gap without changing either controller's
command set.  The VM-free FDC fixture must drive its existing non-DMA command
and result phases through a frozen media registry and exercise provider query,
read, write, and format callbacks.  The VM-free ATA fixture must drive its
existing PIO read and write phases through the same kind of registry.  Each
fixture records copied callback data, typed results, and generation observations;
it never borrows a VM backing object or adds a test-only execution route.

The media contract has no partial-success or `short read` result: a provider
either completes the requested byte range or returns a typed failure.  A short
or out-of-range request is represented by `invalid-range`; absent, read-only,
transient, and permanent failures retain their existing typed meanings.  FDC
maps read/write/format failures to its established result bytes, while ATA
maps an invalid range to ID-not-found and the other admitted provider failures
to abort.  Generation is observed from copied query results at controller
command boundaries; neither controller keeps a private media cache.

`flush` remains a registry/provider operation, not an admitted FDC or ATA
command in this task.  Its direct core-contract corpus verifies its typed
route; a future controller protocol that owns a flush command must add its own
controller-level evidence.  Registry rebind rejection after freeze is likewise
a registry invariant proved by the direct media-provider corpus, while the
FDC/HDC fixtures prove their frozen bindings are the only controller I/O route.

### Core-Only Mantle-Shape Fixture (T274)

T274 is a test-only consumer of existing core contracts, not a new runtime
layer. Its permitted sequence is: create one `core_machine`; borrow only during
the initialized configuration window; bind one fixture-owned execution provider
that advances a fixture-owned `core_machine_rtc`; bind and freeze a
fixture-owned `core_machine_media_registry`; initialize an independent opaque
`core_platform_backing_resource`; freeze the machine; reset; atomically apply
one copied entry plan; then run bounded slices through `core_machine_run`.

The fixture owns every fake provider/context and destroys them only after the
machine has stopped and been destroyed. It asserts typed lifecycle/run results
and copied state only. It includes no VM header, PC/AT port/IRQ default,
firmware, UI, DOS vocabulary, host path/handle, session manager, or second
executor. The backing resource is observed through its synchronous copied API;
it is not connected to a new host I/O path.

## Core Machine: Hardware IRQ

Hardware IRQ delivery and a guest `INT n` instruction are separate mechanisms.
Both software-interrupt routes remain inside the one core CPU executor: real
mode uses ordinary guest-IVT transfer, while the currently admitted protected
mode subset uses a 16-bit IDT interrupt-gate transfer. Its admitted exception
handoff is limited to the corpus-proven `#GP` and `#NP` routes; it is not a
claim of generic protected-mode exception or hardware-interrupt delivery.
The admitted IDT validation error code uses `EXT=0`, because these routes are
synchronous CPU events; external interrupt delivery remains outside this scope.
Core has no firmware-interrupt portal, software-interrupt provider, or
profile-private decoder bypass.

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

Filesystem product policy, drive visibility, serial/parallel policy, and
printing do not enter `core/platform` merely because they touch the host. T271
admits only the underlying opaque file/directory/stream primitives because the
known external-consumer evidence establishes reuse. It exposes no DOS path,
drive-letter, wildcard, sandbox, sharing, lock, mount, or UI policy. VM media
attachment and VDM containment remain composition/product decisions;
composition adapts a core/platform resource into a core/machine media result.

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

### VM Machine-Profile Admission

A VM machine profile declares only identity; generic CPU/FPU/RAM and clock
requirements; VM device topology; port/IRQ/DMA bindings; controller defaults;
CMOS defaults; ROM mapping requirements; firmware-hook metadata; abstract
media-slot compatibility; and pure host-input mapping tables. It has no live
machine, controller, session, thread, platform/product handle, mutable media,
local path, CLI setting, or product UX policy. `core` receives generic
configuration and providers only: it never learns a PC model, ROM vendor, ROM
path, media path, or BIOS-service meaning.

VM composition selects and validates one declaration, creates the one core
machine and VM-only objects, installs all provider/ROM/firmware bindings during
`INITIALIZED`, freezes the topology, then resets and runs it. Every profile,
ROM, port/IRQ/DMA route, and frozen clock parameter remains fixed until the
session is destroyed; RAM alone retains its explicit cold-reconfiguration
operation. Providers borrow only narrow immutable profile data at registration,
not a global profile or composition handle.

A future BYOB ROM facility is composition-owned. A profile may declare an
abstract read-only ROM slot and its permitted address/length/entry constraints;
a caller manifest may name one local file, size, SHA-256, slot, mapping, and
provenance statement. Composition validates all of those before it creates the
machine and reports absence, read, size/hash, overlap, mapping, or entry
failure factually. It never downloads, enumerates, catalogues, bundles, or
defaults a vendor ROM. Core sees only an immutable generic mapping provider.

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
