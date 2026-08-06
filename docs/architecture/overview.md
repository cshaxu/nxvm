# Architecture Overview

## Product Shape

ntvdm64 is the canonical successor to the NXVM machine codebase. It plans two
executable products and three independently buildable components:

```text
nxvm.exe
  bootable whole-machine VM with BIOS/POST, disk-image boot, its retained
  interactive NXVM Console, and whole-machine debugging behavior. It has no
  new process CLI.

nxvdm.exe
  non-bootable DOS application runner with an owned DOS backend and
  non-invasive host integration.

core.dll
  shared machine, platform, and product-tool foundation.

mantle.dll
  shared VDM composition mechanism for NXVDM and an admitted external NTVDM
  implementation.

dos.dll
  independent owned DOS implementation with no dependency on core, mantle,
  VM, or VDM.
```

`core.dll`, `mantle.dll`, and `dos.dll` are medium-term targets, not current
independent artifacts. The planned `nxvdm run` contract is defined in
[Runtime CLI Requirements](../requirements/nxvdm-runtime.md). Microsoft NTVDM
components, invasive integration, and Win16 remain research-only unless a later
owner-approved Go decision changes that boundary.

Forward source ownership and dependency decisions are defined exclusively by
[Module Layout](module-layout.md); public interface decisions are defined by
[Contracts](contracts.md). [Coding Standard](../coding-standard.md),
[Source Policy](../source-policy.md), and
[Execution Policy](../planning/execution-policy.md) define the corresponding
local coding, source, and execution rules. Historical records preserve
rationale and evidence, but cannot redefine a current boundary, interface, or
milestone scope.

Use the public [Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md)
for reusable layering, ownership, contract, and abstraction method. This
overview and the local module documents remain the authority for ntvdm64's
dual-product architecture.

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
where `NNNN` is the four-digit monotonic developer-artifact revision allocated
when a runnable task completes. It normally matched the task number while
tasks completed in order, but task IDs remain roadmap identities and artifact
revisions never regress when urgent work completes out of order. For example,
the early T48 artifact was `0.5.0048`; each task record states its final
task-to-revision mapping. Design-only subtasks do not create artifacts by
themselves.

Standalone documentation tasks use the `M<milestone> Td` identifier and do not
allocate `NNNN`, change the current task artifact version, or create an
executable. Design work explicitly attached to an implementation task uses that
task's subtask identifier and follows its artifact policy.

The bootable product keeps the Virtual Machine identity. After M9 implements
the non-bootable DOS runner and proves that `nxvdm.exe` cannot continue into
standalone disk boot or an implicit guest DOS shell, that product uses:

```text
Neko's x86 Virtual DOS Machine [0.5.NNNN] Copyright (c) 2012-2026 Neko.
```

Post-M5 task versions are task-governed developer versions, not release
numbers. A future release cadence may restore NXVM-style `DDDH` release
encoding through a separate owner-approved release policy.

## Shared Foundation

The source components are `core`, `vm`, `mantle`, `dos`, and `vdm`. `core` has
a strictly neutral `core/utils` layer below `core/machine`, `core/platform`,
and `core/product`. `vm/` owns NXVM composition, lifecycle, and teardown.
`mantle/` owns the reusable VDM composition mechanism over core. `dos/` is an
independent DOS implementation. `vdm/` is the NXVDM product shell over mantle
and dos; its `product` module owns user experience rather than composition.
Cross-module adaptation, including display and input bridging, occurs only in
the relevant product root composition. Peer machine and platform modules do
not adapt each other's contracts directly.

`core/product/session` is a shared opaque registry and command facility for
product sessions. It owns neither a concrete VM/VDM session nor composition;
VM and mantle composition provide concrete lifecycle callbacks. No
`core/composition` layer exists or is permitted.

`src/type.h` is the system-wide type, `nxvm_core_status`, retained alias, and
legacy helper foundation. Each product module owns its compile-time name; the
shared `core/product/banner.h` helper supplies the common version, copyright,
build time, and printing format. Public symbols use their ownership path, for
example `core_machine_*`, `vm_product_*`, `mantle_product_*`, `dos_machine_*`,
and `vdm_platform_*`. The detailed
registry and dependency rules live only in
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
