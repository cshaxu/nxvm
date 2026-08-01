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
