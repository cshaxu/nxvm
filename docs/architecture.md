# Architecture

## Product Shape

ntvdm64 is the canonical successor repository for the NXVM machine codebase. It
builds two first-class executables over one shared core:

```text
nxvm.exe
  bootable whole-machine VM with BIOS/POST, disk-image boot, its retained
  interactive NXVM Console, and whole-machine debugging behavior. It has no
  new process CLI.

ntvdm64.exe
  non-bootable DOS application runner with an owned DOS backend and
  non-invasive host integration.
```

The planned `ntvdm64 run` contract is defined in
[Runtime CLI Requirements](requirements/runtime-cli.md). Microsoft NTVDM
components, invasive integration, and Win16 remain research-only unless a later
owner-approved Go decision changes that boundary.

Forward directory and ownership decisions are defined exclusively by
`architecture/module-layout.md`. The canonical roots are `core`, `vm`, `vdm`,
`adapter`, and `nxvm-baseline`; current horizontal roots are migration sources,
not forward modules. The older boundary and directory sections in this document
describe pre-migration context.

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
      vm/*                         vdm/*
       |                             |
       +---------- core/* -----------+
                     |
                  host OS

adapter/*: temporary migration glue
nxvm-baseline/*: independent reference only
```

- `core/machine`: shared guest mechanics and firmware-service registry.
- `core/platform`: shared host-capability contracts and host facilities.
- `core/product`: shared session, registry, debug/trace and tooling primitives.
- `vm/{machine,platform,product,profile}`: bootable NXVM-only behavior;
  built-in BIOS/POST/ROM/QDX lives in `vm/profile/default_profile/firmware`.
- `vdm/{machine,platform,product,profile}`: DOS app-runner-only behavior;
  the owned DOS backend lives in `vdm/machine`.
- `adapter`: temporary concrete bridging only; `nxvm-baseline`: immutable
  regression reference only.

## Registries

Registries are session-owned composition tables, not process-global mutable
maps. A registry entry has a key, versioned contract, owner, profile/capability
gate, lifecycle state, and teardown rule.

- Machine-profile registry: selects bootable NXVM machine descriptions such as
  `nxvm.machine.default_profile_builtin`, including topology, firmware provider, and
  boot/media policy.
- Execution-profile registry: selects non-booting ntvdm64 capability sets such
  as `ntvdm64.execution.dos_minimal`, including owned DOS and platform policy.
- Device registry: owns generic device lifecycle and reset/run participation.
- Port and memory-range registry: maps I/O ports and mapped memory ranges to
  checked callbacks.
- Interrupt registry: maps guest interrupt vectors to Machine, firmware, DOS,
  or tooling handlers.
- Firmware service registry: routes BIOS services such as INT 10h, INT 13h,
  INT 16h, and INT 1Ah to firmware handlers and declared host capabilities.
- Firmware-provider registry: selects built-in, external-ROM, or absent
  providers subject to the selected profile and redistribution policy.
- DOS service registry: routes DOS ABI functions such as INT 20h and INT 21h
  to the owned DOS module.
- Host capability registry: exposes abstract input, display, clock,
  filesystem, block-device, audio, serial/parallel, print, and logging
  providers selected by `runtime`.
- Debug/command registry: exposes synchronized developer and product commands
  without giving Console or window threads direct access to guest state.

## Dependency Rules

Forbidden dependencies are `core -> VM/VDM policy`, `core -> concrete product
UI`, `platform -> guest state`, `vm -> vdm`, `vdm -> vm`, and profile-to-foreign
product implementation. Allowed dependencies are `vm|vdm -> core`,
`core/product -> abstract core/machine and core/platform`, `core/platform ->
host OS`, and `adapter -> concrete transition interfaces`.

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
The initial M3 contracts did not move the real whole-PC executor. M5 owns its
source migration according to `docs/planning/m5-execution-migration.md`.

The full baseline preserves the existing Linux platform implementation
alongside Win32. M1 acceptance is the Windows GCC run; Linux is kept as a source
baseline for a future platform provider.

## Directory Plan

```text
src/
  core/{machine,platform,product}/
  vm/{machine,platform,product,profile}/
  vm/main.c
  vdm/{machine,platform,product,profile}/
  vdm/main.c
  adapter/
  nxvm-baseline/
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

Headers live beside their C implementation in the owning module. Public headers
use ordinary module names such as `src/core/machine/machine.h`; private headers
use an `_impl.h` suffix. A future SDK or library packaging task may introduce a
top-level `include/` tree after the ABI is stable.

Directory README files define ownership when the directories are created.
Historical sources provide orientation; tests provide validation; `vm` preserves
NXVM's whole-machine value and `vdm` provides the DOS product value.
