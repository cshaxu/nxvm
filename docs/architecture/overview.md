# Architecture Overview

## Product Shape

ntvdm64 is the canonical dual-product successor to the NXVM machine codebase.
It builds two first-class executables over one shared foundation:

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
[Runtime CLI Requirements](../requirements/ntvdm64-runtime.md). Microsoft NTVDM
components, invasive integration, and Win16 remain research-only unless a later
owner-approved Go decision changes that boundary.

Forward source ownership and dependency decisions are defined exclusively by
[Module Layout](module-layout.md); public interface decisions are defined by
[Contracts](contracts.md). Historical records preserve rationale and evidence,
but cannot redefine a current boundary, interface, or milestone scope.

## Runtime Identity And Versioning

The immutable M1 byte-identical NXVM snapshot retains its imported startup
banner exactly:

```text
Neko's x86 Virtual Machine [0.4.015d]
Copyright (c) 2012-2014 Neko.
```

After that snapshot and until the product split is implemented, a task-level
machine artifact uses:

```text
Neko's x86 Virtual Machine [0.4.015d.m<M>t<T>]
Copyright (c) 2012-2014 Neko.
```

`M` and `T` are the decimal milestone and task identifiers that produced the
executable, for example `0.4.015d.m3t1`. Subtask-specific executables are not
developer artifacts.

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

## Shared Foundation

The product forms are `core`, `vm`, and `vdm`. The shared foundation has three
independent modules: `core/machine`, `core/platform`, and `core/product`.
`vm/` and `vdm/` each own product composition, lifecycle, and teardown; their
`product` modules own user experience rather than system composition.
Cross-module adaptation, including display and input bridging, occurs only in
the relevant product root composition. Peer machine and platform modules do
not adapt each other's contracts directly.

`src/type.h` is the system-wide type and `STATUS` foundation. `src/version.*`
is the only source for version and build identity. Public symbols use their
ownership path, for example `core_machine_*`, `vm_product_*`, and
`vdm_platform_*`. The detailed registry and dependency rules live only in
[Module Layout](module-layout.md) and [Contracts](contracts.md).

## Research Boundary

Microsoft component work is not a runtime module or committed backend. It
belongs under `docs/research/` and owner-approved `tools/research/` utilities.
It cannot create a BOP framework, component loader, default dependency, or
release requirement before M11 research and an owner-approved Go decision.

## Baseline Record

M1 established the runnable whole-NXVM baseline before refactoring. M5
migrated the executor, devices, firmware, presentation, retained Console, and
debugger into the canonical roots. Git history and provenance records preserve
the baseline evidence. Windows GCC is the acceptance run; the historical Linux
platform source remains a future portability asset until a Linux run gate is
introduced.
