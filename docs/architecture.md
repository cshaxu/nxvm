# Architecture

## Product Shape

ntvdm64 evolves NXVM machine code into a DOS and Win16 compatibility runtime
for 64-bit Windows. The primary product path is non-invasive command-line
execution:

```text
ntvdm run <program> [args]
```

The default backend is a project-owned DOS layer. Optional Microsoft guest mode
is experimental and BYOB. Win16 is research-only until a separately approved
route is selected.

## Module Boundaries

```text
app -> runtime -> adapters -> dos + platform
                               |       |
                            machine   host OS
```

- `machine/`: NXVM-derived CPU, memory, BIOS, I/O bus, devices, debugger, and
  machine runner. It has no DOS or Windows API dependency.
- `dos/`: project-owned loader, PSP, environment, MCB, interrupts, process,
  filesystem, console, and later XMS/EMS/DPMI behavior. It consumes abstract
  machine and host-service interfaces.
- `platform/`: non-invasive host adapters. Windows-specific filesystem,
  console, input, display, timing, process, and logging code stay here.
- `integration/`: isolated research for host-changing features. It is excluded
  from the default build and release.
- `adapters/`: explicit glue between concrete machine, DOS, and platform
  implementations. Cross-module policy does not live inside a core module.
- `runtime/`: composition root that creates modules, configures adapters,
  drives the execution loop, and reports results.
- `app/`: CLI commands only; it contains no emulation logic.
- `microsoft/`: future BYOB guest profile, hash validation, loader, and BOP
  adapter work. It never contains Microsoft binary files.

## Dependency Rules

Forbidden dependencies are `machine -> dos`, `machine -> Windows`, `dos ->
concrete Windows APIs`, `platform -> DOS internals`, and `integration ->
machine internals`. Allowed dependencies are `dos -> abstract machine and host
interfaces`, `platform -> host OS`, `adapters -> concrete interfaces`, and
`runtime -> major modules`.

## Directory Plan

```text
src/
  app/ runtime/ machine/ dos/ platform/ adapters/ microsoft/ integration/
tests/
  machine/ dos/ platform/ adapters/ runtime/ microsoft/ integration/
```

Directory README files define future ownership. This change does not copy, move,
or alter NXVM runtime code. NXVM assimilation begins only under a tracked
subtask with a provenance record and preserved license notices.

## Backends

```text
DOS program -> owned DOS kernel -> machine -> platform
DOS program -> optional Microsoft guest -> BOP adapter -> machine + platform
```

The first route is required for all supported releases. The second route may
increase compatibility but is optional, profile-bound, and unable to block the
first. Historical sources provide orientation; tests provide validation; NXVM
provides the machine foundation; ntvdm64 provides the new integration.
