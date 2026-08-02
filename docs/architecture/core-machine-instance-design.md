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

Every mutable guest object belongs to exactly one `core_machine` session. The
session owns CPU/executor state, RAM/A20, port routing, PIC, PIT, DMA, CMOS,
KBC, video-adapter state, trace state, and session-local registries. A device can
own private child state, but its lifetime remains bounded by its parent
session. No process-global variable may select the active session.

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
| Block I/O | drive registry, drive identity, request validation | VM image or VDM contained-filesystem provider | `block_registry_interface.h`, `block_provider.h` |
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

## Object Ownership Ledger

| Subject | Private state and implementation | Exposed capability | Injected provider | Current gap |
| --- | --- | --- | --- | --- |
| machine | session lifecycle, run result, stop flag, child ownership | create, freeze, reset, run, stop, destroy | none | current minimal `core_machine` is not the retained full-PC executor |
| CPU/executor | registers, decoder scratch, instruction helpers | copied state, bounded run, controlled debug | none | `vcpu` and `vcpuins` choose one global live object |
| memory | RAM allocation, A20 and physical translation | copied physical read/write, A20 control | mapped-memory provider only if a future profile needs one | legacy real-mode helpers use `vram` |
| port bus | port map, conflict/width/fault rules | port install/read/write | port device provider | legacy `vport` remains selected globally |
| PIC/PIT/DMA | controller registers and IRQ/DMA state | device diagnostics only when a consumer needs them | profile device configuration, not replacement dispatch rules | all use global live aliases |
| CMOS | RTC/NVRAM state and reset policy | optional diagnostic/configuration view | profile CMOS initialization provider | current mutable model remains VM-private |
| KBC | controller state and guest ingress queue | guest input injection and optional diagnostics | profile keyboard-route provider | `vkbc` is a global live alias |
| VADP | guest video state and dirty/generation state | copied text/graphics snapshot | profile video snapshot provider | `vvadp` and global display callbacks select one session |
| firmware | frozen service registry and dispatch bookkeeping | register/freeze/query/dispatch | profile firmware provider | descriptor registry is external and has no callbacks |
| block | mounted-drive registry and request validation | geometry/read/write by explicit drive id | VM image or VDM filesystem block provider | one global, drive-zero-only CHS provider |
| trace | event buffer, sequence, flush guard | copied trace/event subscription | trace observer | close to target; provider is already session-bound |
| debug | paused-state inspection and bounded control | read CPU/RAM, step, continue | none; product debugger adapts it | blocked by executor's global context |
| presentation data | copied text snapshot and keyboard queue helpers | pure value operations | none | already independent; do not turn it into UI ownership |

The ledger deliberately distinguishes profile configuration from a provider.
For example, a PC/AT profile may configure two PICs and their ports, but it
cannot replace core's port conflict or interrupt-delivery rules. A VM FDC may
provide port callbacks, but it cannot create a second port bus.

## Firmware Design

Firmware has two separate roles and therefore two separate contracts.

`firmware_registry_interface.h` is a core-owned, session-owned registry for
POST, ROM, and interrupt services. It defines registration order, duplicate
service and vector rejection, freeze, lookup, interrupt dispatch order, chain
transfer, and failure result. It stores service descriptors and provider
bindings, not product-specific BIOS code.

`firmware_provider.h` is supplied by a VM or VDM profile. It provides explicit
callbacks such as service composition, ROM image installation, reset/POST, and
interrupt handling. The provider context belongs to the same session and must
remain valid through teardown. Default PC/AT BIOS code stays under
`vm/profile/default_profile/firmware`; future DOS-minimal firmware stays under
`vdm/profile`.

The existing `firmware_interface.h` is an incomplete registry contract: it
describes services but exposes no callable provider. It is a transition target
for the two-file split above, not a file to rename blindly into
`firmware_provider.h`.

## Current-State Audit

The following units already use a `core_machine *` instance correctly or are
close enough to retain their direction:

- `machine_interface`, `memory_interface`, `port_interface`,
  `trace_interface`, and `debug_interface` use or are designed around an
  explicit machine handle.
- memory, port routing, and trace buffering are stored in the current minimal
  `core_machine` object.
- `presentation_interface` contains standalone copied text/queue data and has
  no selected-machine global.

The following units currently violate the target because they retain a
process-global live pointer or provider binding:

| Unit | Current transitional form | Required outcome |
| --- | --- | --- |
| CPU and instruction executor | `core_machine_cpu_current()` and `vcpu`/`vcpuins` aliases | executor functions receive the owning machine/execution context |
| RAM, port, PIC, PIT, DMA, KBC, VADP | `*_bind_live()` plus `vram`, `vport`, `vpic*`, `vpit`, `vdma*`, `vkbc`, `vvadp` aliases | state becomes a child of one `core_machine`; temporary aliases are removed only after callers pass context |
| block | one static geometry/read/write provider and no drive identity | session block registry with multiple drive/provider entries |
| keyboard | one static provider/context pair | session keyboard provider slot or registry |
| display | two static provider/context pairs | session display observer and snapshot contracts |
| firmware | external descriptor object, no provider contract | session firmware registry plus profile provider bindings |

The existing VM live-machine carrier proves that retained objects have only one
storage location. It does not yet solve selected-session globals: the carrier
itself and all legacy accessors still select one process-wide live machine.

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
rules, then migrate it without changing user-visible behavior. M6 remains
deferred until the selected-session globals and the dual machine execution
paths have a bounded M5 plan.

The approved execution order and per-task deletion gates are recorded in
[M5 Instance Authority Plan](../planning/m5-instance-authority-plan.md).
