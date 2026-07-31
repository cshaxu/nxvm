# Architecture

## Product Shape

ntvdm64 is the canonical successor repository for the NXVM machine codebase. It
builds two first-class executables over one shared core:

```text
nxvm.exe
  bootable whole-machine VM with BIOS/POST, disk-image boot, Console and
  whole-machine debugging behavior.

ntvdm64.exe
  non-bootable DOS application runner with an owned DOS backend and
  non-invasive host integration.
```

The planned `ntvdm64 run` contract is defined in
[Runtime CLI Requirements](requirements/runtime-cli.md). Microsoft NTVDM
components, invasive integration, and Win16 remain research-only unless a later
owner-approved Go decision changes that boundary.

## Runtime Identity And Versioning

The immutable M1 byte-identical NXVM snapshot retains its imported startup
banner exactly:

```text
Neko's x86 Virtual Machine [0.4.015d]
Copyright (c) 2012-2014 Neko.
```

After that snapshot and until the product split is implemented, a standalone
machine artifact uses:

```text
Neko's x86 Virtual Machine [0.4.015d.m<M>t<T>s<S>]
Copyright (c) 2012-2014 Neko.
```

`M`, `T`, and `S` are the decimal milestone, task, and subtask identifiers that
produced the executable. A task-level aggregate omits `s`, for example
`0.4.015d.m3t1`; a subtask-specific executable is not a developer artifact.

After M5 implements the first-class `nxvm.exe` surface, that product keeps the
Virtual Machine identity and follows the NXVM versioning lineage. After M9
implements the non-bootable DOS runner and proves that `ntvdm64.exe` cannot
continue into standalone disk boot or an implicit guest DOS shell, that product
uses:

```text
Neko's x86 Virtual DOS Machine [0.5.0000] Copyright (c) 2012-2026 Neko.
```

Post-cutover ntvdm64 versions use NXVM-style `DDDH` encoding: `DDD` is a
zero-padded three-digit decimal release counter and `H` is one lowercase
hexadecimal revision digit (`0` through `f`).

## Module Boundaries

```text
products/nxvm      products/ntvdm64
      |                   |
      +-------- runtime --+
                 |
      +----------+----------+
      |          |          |
    core     firmware      dos
      |          |          |
      +------ adapters -----+
                 |
              platform
                 |
              host OS
```

- `core/`: CPU, RAM, bus, port dispatch, memory map, interrupt controller
  contracts, generic device lifecycle, trace, debug, profile hooks, and
  `Machine` instance state. It has no DOS, firmware policy, product CLI, or
  host OS dependency.
- `firmware/`: BIOS/POST/ROM behavior and BIOS interrupt/service handlers. It
  can be composed into `nxvm.full_pc` and selected ntvdm64 profiles through the
  firmware service registry, but it does not own product CLI or host handles.
- `platform/`: host capability providers such as Win32, retained Linux source,
  future macOS, display, input, clocks, block files, host filesystem, audio,
  logging, and process/Console integration. It does not know DOS internals.
- `dos/`: ntvdm64-only loader, PSP, environment, DTA, handles, DOS service
  registry, fixture filesystem for early tests, and later DOS compatibility
  behavior. It consumes abstract Machine and host capability contracts.
- `runtime/`: the composition root. It creates a session, selects a product
  profile, wires registries, owns lifecycle transitions, drives execution, and
  reports a product result.
- `products/nxvm/`: bootable VM CLI, Console, boot media policy, whole-machine
  profile selection, and nxvm-specific registry composition.
- `products/ntvdm64/`: DOS app-runner CLI, display/debug UX, drive visibility,
  host filesystem policy, and ntvdm64-specific registry composition.
- `adapters/`: explicit glue between concrete modules. Cross-module policy does
  not live inside `core`.
- `integration/`: isolated research for host-changing features; excluded from
  default builds and releases.

## Registries

Registries are session-owned composition tables, not process-global mutable
maps. A registry entry has a key, versioned contract, owner, profile/capability
gate, lifecycle state, and teardown rule.

- Profile/composition registry: selects `nxvm.full_pc`,
  `ntvdm64.dos_minimal`, test profiles, and enabled module sets.
- Device registry: owns generic device lifecycle and reset/run participation.
- Port and memory-range registry: maps I/O ports and mapped memory ranges to
  checked callbacks.
- Interrupt registry: maps guest interrupt vectors to Machine, firmware, DOS,
  or tooling handlers.
- Firmware service registry: routes BIOS services such as INT 10h, INT 13h,
  INT 16h, and INT 1Ah to firmware handlers and declared host capabilities.
- DOS service registry: routes DOS ABI functions such as INT 20h and INT 21h
  to the owned DOS module.
- Host capability registry: exposes abstract input, display, clock,
  filesystem, block-device, audio, serial/parallel, print, and logging
  providers selected by `runtime`.
- Debug/command registry: exposes synchronized developer and product commands
  without giving Console or window threads direct access to guest state.

## Dependency Rules

Forbidden dependencies are `core -> dos`, `core -> platform/host OS`, `core ->
product CLI`, `dos -> concrete platform APIs`, `platform -> DOS internals`, and
`products/* -> core internals`. Allowed dependencies are `firmware -> abstract
Machine and host capability contracts`, `dos -> abstract Machine and host
capability contracts`, `platform -> host OS`, `adapters -> concrete module
interfaces`, and `runtime -> major modules`.

All guest-state mutations occur on the Machine execution thread at a command
boundary. Platform threads exchange timestamped input events and immutable
display snapshots through adapters. Trace callbacks receive copied event data
and never a mutable Machine pointer.

## Research Boundary

Microsoft component work is not a runtime module or committed backend. It
belongs under `docs/research/` and owner-approved `tools/research/` utilities.
It cannot create a BOP framework, component loader, default dependency, or
release requirement before M11 research and an owner-approved Go decision.

## M1 Baseline Exception

M1 first establishes a runnable whole-NXVM baseline before subtractive
refactoring. Its imported source may temporarily live in
`src/nxvm-baseline/`, retaining upstream machine and platform coupling solely
to reproduce recorded baseline behavior. No new product feature belongs there.
M3 moves retained code into the final boundaries after M2 closes.

The full baseline preserves the existing Linux platform implementation
alongside Win32. M1 acceptance is the Windows GCC run; Linux is kept as a source
baseline for a future platform provider.

## Directory Plan

```text
src/
  nxvm-baseline/
  core/
  firmware/
  platform/
  dos/
  runtime/
  adapters/
  products/
    nxvm/
    ntvdm64/
  integration/
tests/
  core/
  firmware/
  platform/
  dos/
  runtime/
  products/
tools/
  research/
docs/
  requirements/
  planning/
  provenance/
  verification/
  tracking/
  research/
```

Headers live beside their C implementation in the owning module. Contract
headers use ordinary module names such as `src/core/machine.h`; private headers
use an `_impl.h` suffix and are included only by their module. A future SDK or
library packaging task may introduce a top-level `include/` tree after the ABI
is stable, but M3 uses parity-first layout.

Directory README files define ownership when the directories are created.
Historical sources provide orientation; tests provide validation; `core` and
`firmware` preserve NXVM's whole-machine value; `dos` and
`products/ntvdm64` provide the VDM product value.
