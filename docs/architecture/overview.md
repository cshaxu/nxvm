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

`RULES.md` is the compact repository-wide implementation/review checklist.
Forward source ownership and dependency decisions are defined exclusively by
[Module Layout](module-layout.md); public interface decisions are defined by
[Contracts](contracts.md). Historical records preserve rationale and evidence,
but cannot redefine a current boundary, interface, or milestone scope.

## Runtime Identity And Versioning

The immutable M1 byte-identical NXVM snapshot retains its imported startup
banner exactly:

```text
Neko's x86 Virtual Machine [0.4.015d]
Copyright (c) 2012-2026 Neko.
```

Historical M3 through M5 T47 task-level machine artifacts used:

```text
Neko's x86 Virtual Machine [0.4.015d.m<M>t<T>]
Copyright (c) 2012-2026 Neko.
```

`M` and `T` are the decimal milestone and task identifiers that produced the
executable, for example `0.4.015d.m3t1`. Subtask-specific executables are not
developer artifacts.

Beginning with M5 T48, task artifacts use the NXVM lineage `0.5.NNNN` form,
where `NNNN` is the four-digit decimal task number. For example, T48 is
`0.5.0048` and its bootable VM developer artifact is
`build/output/nxvm_0_5_0048.exe` when T48 completes. Design-only subtasks do
not create artifacts by themselves.

The bootable product keeps the Virtual Machine identity. After M9 implements
the non-bootable DOS runner and proves that `ntvdm64.exe` cannot continue into
standalone disk boot or an implicit guest DOS shell, that product uses:

```text
Neko's x86 Virtual DOS Machine [0.5.NNNN] Copyright (c) 2012-2026 Neko.
```

Post-M5 task versions are task-governed developer versions, not release
numbers. A future release cadence may restore NXVM-style `DDDH` release
encoding through a separate owner-approved release policy.

## Shared Foundation

The product forms are `core`, `vm`, and `vdm`. The shared foundation has three
independent modules: `core/machine`, `core/platform`, and `core/product`.
`vm/` and `vdm/` each own product composition, lifecycle, and teardown; their
`product` modules own user experience rather than system composition.
Cross-module adaptation, including display and input bridging, occurs only in
the relevant product root composition. Peer machine and platform modules do
not adapt each other's contracts directly.

`core/product/session` is a shared opaque registry and command facility for
product sessions. It owns neither a concrete VM/VDM session nor composition;
VM and VDM composition provide concrete lifecycle callbacks. No
`core/composition` layer exists or is permitted.

`src/type.h` is the system-wide type, `nxvm_core_status`, retained alias, and
legacy helper foundation. `src/version.*` is the only source for version and
build identity. Public symbols use their ownership path, for example
`core_machine_*`, `vm_product_*`, and `vdm_platform_*`. The detailed registry
and dependency rules live only in
[Module Layout](module-layout.md) and [Contracts](contracts.md).

`core/machine` session ownership plus provider/registry naming rules are
defined in [Core Machine Instance Design](core-machine-instance-design.md).

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
