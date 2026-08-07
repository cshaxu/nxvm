# Core Machine Instance Design

## Authority

This document is the forward-looking authority for `core/machine` naming,
object ownership, public contracts, and injected providers. It supersedes only
the forward-looking parts of historical M3--M5 authority records; those
records remain verification evidence.

The design applies before any context-passing implementation work. It does not
authorize a change to NXVM Console, debugger UX, boot/reset order, guest media
behavior, or CPU behavior.

## Terms And File Names

A **machine session** is one independently resettable guest machine. Its root
object is `core_machine`. A session may be created by VM or VDM root
composition, but `core/machine` does not know which product created it.

- `<subject>.c` and `<subject>.h` implement private machine state and helpers.
  They are not cross-module contracts.
- `<subject>_interface.h` is the minimal capability that `core/machine`
  exposes to another module. It accepts an explicit `core_machine *` or a
  session-owned child object; it never resolves a current global machine.
- `<subject>_provider.h` defines callbacks supplied from outside the machine.
  A provider is `{ callbacks, context }`, belongs to one session, and is
  installed by root composition during the configuration phase.
- `<subject>_registry_interface.h` exposes a session-owned registry when more
  than one provider/service can be selected or queried. A registry validates
  identity, rejects conflicts, and freezes before execution.
- A **descriptor** is declarative data. It is not a provider unless it carries
  callable behavior through an explicit provider contract.

Public symbols follow their owner path: `core_machine_*`, `vm_machine_*`,
`vdm_machine_*`, and so on. A provider callback type ends in `_provider` and
its installation API ends in `_install_provider` or `_bind_provider`.
Compatibility aliases may retain legacy names temporarily, but are private
implementation debt and must never appear in a new cross-module contract.

## Session Ownership

Every mutable core guest object belongs to exactly one `core_machine` session.
The session owns CPU/executor state, RAM/A20, port routing, PIC, PIT, DMA, KBC,
video-adapter state, trace state, and session-local registries. A device can own
private child state, but its lifetime remains bounded by its parent session. No
process-global variable may select the active session. T273 may add an optional
core-owned RTC controller; until then RTC state remains VM-owned.

Immutable tables may be process-global only when they contain no guest state,
provider context, profile selection, host handle, trace cursor, or resettable
state. Examples include opcode metadata and static default-profile descriptor
data. Mutable provider contexts and all callback registrations are session
owned.

Root composition is the sole assembler. It creates a `core_machine`, creates
VM- or VDM-specific state, selects a profile, installs providers, freezes
registries, then drives bounded synchronous execution. It alone owns threads,
host event loops, pacing, teardown, and product exit policy.

## Capability Families

| Family | Core owns | Outside supplies | Target public surface |
| --- | --- | --- | --- |
| CPU/executor | registers, decoder scratch, execution state | none for ordinary execution | `cpu_interface.h`, `debug_interface.h` |
| Memory | RAM, A20, address checking | none | `memory_interface.h` |
| Ports | routing table and conflict policy | port read/write providers | `port_interface.h`, `port_provider.h` |
| Interrupt devices | PIC/PIT/DMA/KBC/VADP mutable models | profile/device configuration only | device-specific interfaces when externally needed |
| Trace | event buffer, ordering, flush guard | trace observer | `trace_interface.h`, `trace_provider.h` |
| Firmware | service registry, ordering, vector-conflict and dispatch rules | profile firmware services and callbacks | `firmware_registry_interface.h`, `firmware_provider.h` |
| Media I/O | frozen device registry and request validation | composition-owned media provider | T270 `media_provider` contract; it is not a host-filesystem API |
| Host capabilities | no guest or product policy | opaque file/directory/stream/clock/input/cancellation providers | T271 `core/platform` contracts; composition adapts them to product or media policy |
| Keyboard ingress | guest-side routing and queue/state | profile/device keyboard route provider | `keyboard_interface.h`, `keyboard_provider.h` |
| Display egress | guest display state/snapshot contract | profile/device video snapshot provider | `display_interface.h`, `display_provider.h` |

`core_machine` defines validation, callback order, fault conversion, teardown,
and freeze rules. A profile or platform provider implements a capability but
cannot privately change those rules.

Keyboard and display cross the host boundary only through root composition.
Platform event sources produce copied input events; composition applies them at
an execution boundary through `keyboard_interface.h`. Composition later pulls
a copied display snapshot through `display_interface.h`, converts it to a
platform frame, and submits it. Core never calls a platform UI callback, and a
platform callback never mutates a machine.

## Current Ownership Ledger

| Subject | Current owner | Boundary and next action |
| --- | --- | --- |
| machine, CPU, memory, ports, PIC/PIT/DMA/KBC/VADP | `core_machine` | One session-owned executor and device graph; composition configures it before freeze and drives bounded `core_machine_run`. |
| RTC/CMOS controller | `core/machine/rtc.*` | T273 moved the neutral MC146818 mechanism to core; VM retains PC/AT defaults, NMI glue, host-time policy, and firmware. |
| FDC controller | `core/machine/fdc.*` | `core_machine.fdc` owns the neutral controller. |
| ATA PIO controller | `core/machine/hdc.*` | `core_machine.hdc` owns the neutral controller. |
| FDD/HDD backing, paths, mount/eject, persistence | VM composition and `vm/machine` backing objects | T270 exposes device-level media operations; T271 supplies only opaque host primitives. Paths and product policy remain in composition. |
| firmware and profile declaration | `vm/profile/default_profile` plus VM composition | Profile content stays VM-owned; it binds only public core contracts. |
| trace/debug/presentation | core state with VM product/platform adapters | Adapters observe or command an explicit session; no selected-machine facade is permitted. |

The ledger deliberately distinguishes profile configuration from a provider.
For example, a PC/AT profile may configure two PICs and their ports, but it
cannot replace core's port conflict or interrupt-delivery rules. A VM FDC may
provide port callbacks, but it cannot create a second port bus.

## Firmware Design

Firmware has two separate roles and therefore two separate contracts.

Firmware remains profile-owned content and provider bindings. The obsolete
generic core firmware registry was removed because it had no production
consumer; the default PC/AT path is assembled from the profile descriptor and
`vm/composition/session/profile_firmware.c`. Future profile firmware contracts
must expose only the provider bindings actually consumed by composition, not a
second registry beside the ordinary IVT/ROM path.

## Current-State Audit

The executor migration is complete: CPU, memory, port routing, shared devices,
and run results use the explicit `core_machine` instance in the retained NXVM
route. The remaining second-boundary work is not a selected-session-global
cleanup. It is the narrower controller/media ownership correction recorded in
T270--T278. The retained fixed block slot was removed because it represented
only one CHS device and could not be the shared FDC/ATA boundary; T270 replaced
it rather than layering a forwarding facade over it.

## Migration Invariants

1. One authority move at a time. A task changes one storage/callback family and
   preserves initialization, reset, refresh, finalization, IRQ, DMA, and
   firmware callback order.
2. No duplicated guest state. Compatibility code may alias the same instance;
   it may not mirror, cache, synchronize, or separately reset it.
3. No implicit session. New public APIs take an explicit machine or child
   context. A `*_current()` accessor is transitional and cannot be added to.
4. Configuration precedes execution. Provider installation is allowed only
   while configuring; registry mutation after freeze is rejected.
5. The core does not import VM, VDM, profile, platform, or product code.
6. Root composition converts product/platform events at a defined command or
   execution boundary; platform code does not mutate guest state directly.
7. Every runnable migration task builds its task-numbered artifact and passes
   GCC, dependency-DAG, retained Console/debugger, and FDD DOS-prompt gates.

## T64 Design Output

M5 T64 establishes the naming rule and the ownership ledger above. It is
design-only. Subsequent implementation tasks must first map the exact legacy
callers for one family, define its context-passing API and freeze/lifetime
rules, then migrate it without changing user-visible behavior. Later mantle
work depends on completed M5 single-session and single-execution-path evidence.

The completed migration is summarized in [M5 History](../history/m5.md).
