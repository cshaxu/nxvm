# System Architecture

This is the current high-level architecture authority. Apply the local
[Architecture Rules](../rules/ARCHITECTURE.md) when changing it. Detailed M5
contracts, migration rationale, and hardware notes are supporting material
indexed in [etc/README.md](../etc/README.md); they cannot override this file.

## Product Shape

The NXVM repository has one current product, one future product, and three
planned reusable components:

| Form or component | Purpose | Current state |
| --- | --- | --- |
| `nxvm.exe` / `vm` | Bootable whole-machine VM with the retained NXVM Console and debugger. | Current primary, runnable product. |
| `core` | Product-neutral machine and reusable product tooling. | Shared source foundation; not yet a separate artifact. |
| `mantle` | Policy-free VDM session composition over core and admitted runtime adapters. | Future component. |
| `dos` | Owned DOS implementation independent of the other components. | Future component. |
| `nxvdm.exe` / `vdm` | Non-bootable DOS application runner over mantle and dos. | Future secondary product; only a non-runnable skeleton exists. |

The medium-term build targets are `core.dll`, `mantle.dll`, and `dos.dll`.
They are architectural commitments, not current release artifacts.

## Modules, Ownership, And Assembly

`core` contains independent `machine` and `product` modules. `src/lib/types`
is the shared C type/status foundation. `src/common/xasm32` is the shared x86
assembler/disassembler component; `src/common/debug` is the shared Debug CLI
provider. `src/common/session`, `src/common/machine` and `src/common/ui` own
the shared product runtime mechanisms. `vm` uses `events`, `machine`,
`product`, and `profile` modules.
`mantle` uses `machine`, `platform`, and `product`;
`dos` may use its own `machine`, `platform`, `product`, and `profile` modules.

```text
vm -------> core
mantle ---> core
vdm ------> mantle + dos
dos ------> (independent)
```

`core` owns generic guest-machine behavior and policy-free host abstractions.
It never depends on `vm`, `mantle`, `dos`, or `vdm`.

`vm` owns bootable-machine composition, BIOS/POST boot policy, VM profiles, and
the retained NXVM product experience. `mantle` owns reusable VDM composition,
including the neutral adapter boundary for an owned or separately admitted
runtime, but no DOS or host-policy decision. `dos` owns DOS behavior without
depending on another product component. `vdm` owns application-runner UX and
combines an admitted mantle session with an admitted DOS implementation.

The `vm` and `mantle` roots assemble their concrete sessions. The `vdm` root
selects application-runner UX and binds mantle to dos. Product-root composition
is where the declared machine, platform, product, and profile capabilities are
combined.

Common Debug owns debugger command parsing, assembly/disassembly and its
continuation state; it is a CLI provider registered with common session, not a
machine-state owner. Common machine exposes synchronous bounded paused-Debug
operations through its product driver. NXVM routes those operations through
`vm/machine` to Core machine; SoftPC may bind its own VM/MVDM driver. Core
machine owns
shared instruction decode and execution, checked memory and port
access, and the CPU/DMA transaction lifecycle. Its current specification-driven L3 instruction and transaction timing direction, L2 fallback discipline, and future Core-to-VM timing-plan boundary are detailed in [Specification-Driven Instruction And Transaction Timing Simulation](../etc/architecture/specification-driven-l3-timing.md). A VM machine profile composes
those mechanisms with a documented CPU and board contract; it may add only a
real architectural or physical difference, including CPU feature gates,
address/operand width, protection or paging behavior, bus width, prefetch,
availability/wait rules, and selected device topology.

Equivalent profile routes reuse the core owner and transaction path. The IBM
PC/XT 5160-268 8088 profile reuses 8086-class instruction semantics but owns
its distinct external-bus and prefetch/timing contract. It must not duplicate
opcode execution or create another CPU/DMA memory-or-port transaction path. A
80286 or 80386 profile similarly adds only its documented architectural and
board-local differences through the same core ownership boundaries.

## Product And Host Boundary

Platform integrations report through opaque core contracts. Host policy and
guest-state mutation occur at the owning product composition boundary, never
inside a generic platform implementation.

### NXVM Machine Execution And Presentation

NXVM separates one machine's Core execution from product control.
`common/machine` owns the one ordered safe-point FIFO. It accepts only copied
input and lifecycle requests and calls its opaque product driver at execution
boundaries. `vm/machine` is NXVM's sole driver and Core assembly owner: it
maps those copied requests to Core, publishes copied lifecycle, fault and
display facts, and exposes bounded paused-Debug operations through the common
lease contract. Neither owner selects a host surface or owns the process
Console. `vm/events` remains a value-only display/input ABI below product
owners: it contains no Core, executor, session, or UI pointer.

`common/session` is the sole product-control reducer. Its one FIFO receives
copied Console lines, machine results and presentation input; it owns run
generation and lifecycle facts. `vm/app` is the NXVM composition binding: it
converts the existing VM machine result into copied common facts and contains
no second FIFO, run generation, lifecycle reducer or presentation state.
Product Console retains NXVM command and title policy until the later
common-debug/product-CLI migration.

`vm/presentation` is the NXVM shared-lib binding leaf. It applies common-session plans
and returns copied input facts; it never derives lifecycle or surface policy.
Its Console host owns the process Console broker and may lease it to at most one
Console presenter. Core and `src/lib` do not know session selection, lifecycle
policy, or Console leasing.

Native and WASM hosts share these component boundaries. A future TypeScript web
product layer sits above the WASM platform/product adaptation; it does not move
browser, network, or storage policy into generic machine behavior.

## Runtime Admission Boundary

The owned DOS backend is the default NXVDM direction. A separately admitted
external VDM/DOS implementation is isolated behind a dedicated adapter and
does not become a shared public ABI, default runtime dependency, or release
input. Source, firmware, research, and redistribution procedures are defined
by [Architecture Rules](../rules/ARCHITECTURE.md) and the indexed
[source policy](../etc/operations/policy/source-policy.md).

Current delivery state and staged implementation goals are defined only by
[ROADMAP.md](ROADMAP.md) and [CURRENT.md](../states/CURRENT.md).
