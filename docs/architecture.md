# Architecture

## Product Shape

ntvdm64 evolves NXVM machine code into a DOS compatibility runtime for 64-bit
Windows. The primary product path is non-invasive command-line execution:

```text
ntvdm64 run [options] [<program> [args...]]
```

The command-line contract, including display backend selection and host-drive
visibility, is defined in [Runtime CLI Requirements](requirements/runtime-cli.md).

The default backend is a project-owned DOS layer. Microsoft NTVDM components and
Win16 are future research topics, not current runtime backends.

## Runtime Identity And Versioning

The immutable M1 byte-identical NXVM snapshot retains its imported startup
banner exactly:

```text
Neko's x86 Virtual Machine [0.4.015d]
Copyright (c) 2012-2014 Neko.
```

After that snapshot and until the identity cutover, a still-standalone machine
executable uses:

```text
Neko's x86 Virtual Machine [0.4.015d.m<M>t<T>s<S>]
Copyright (c) 2012-2014 Neko.
```

`M`, `T`, and `S` are the decimal milestone, task, and subtask identifiers that
produced the executable, for example `0.4.015d.m3t1s2`. The runtime banner and
any equivalent window title must derive from one build identity; artifact names
do not replace this banner.

The identity cutover occurs only in an approved M7-or-later implementation
subtask whose tests prove all of the following: the default executable has no
direct NXVM Console or disk-boot launch path; ordinary execution selects an
application binary through the product loader; no implicit DOS shell or booted
guest OS can run; and no-program debug mode, if retained, remains paused and
cannot continue into standalone machine execution. At that cutover, the banner
switches exactly to:

```text
Neko's x86 Virtual DOS Machine [0.5.0000] Copyright (c) 2012-2026 Neko.
```

After cutover, the final four version digits use NXVM-style `DDDH` encoding:
`DDD` is a zero-padded three-digit decimal release counter and `H` is one
lowercase hexadecimal revision digit (`0` through `f`). Increment `H` for each
compatible revision; after `f`, increment `DDD` and reset `H` to `0`. A
verification record must state the assigned version and its predecessor.

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
  machine and host-service interfaces. M5 begins with a bounded, in-memory
  fixture filesystem and deterministic Console interface; direct Win32
  filesystem, Console, and drive behavior belongs to `platform/` in M7.
- `platform/`: non-invasive host adapters. Windows-specific filesystem,
  console, input, display, timing, process, and logging code stay here.
  Windows is the current product target; retained Linux platform code is a
  future portability opportunity, not a current delivery promise.
- `integration/`: isolated research for host-changing features. It is excluded
  from the default build and release.
- `adapters/`: explicit glue between concrete machine, DOS, and platform
  implementations. Cross-module policy does not live inside a core module.
- `runtime/`: composition root that creates modules, configures adapters,
  drives the execution loop, and reports results.
- `app/`: CLI commands only; it contains no emulation logic.

Machine and DOS may both emit optional normalized verification events through a
project-owned abstract trace sink. Trace instrumentation is disabled in ordinary
and release builds; it does not link, inject, or otherwise depend on NTVDMx64.
Differential reference adapters live only in `tools/research/differential/` and
are removed after their bounded validation use.

## Dependency Rules

Forbidden dependencies are `machine -> dos`, `machine -> Windows`, `dos ->
concrete Windows APIs`, `platform -> DOS internals`, and `integration ->
machine internals`. Allowed dependencies are `dos -> abstract machine and host
interfaces`, `platform -> host OS`, `adapters -> concrete interfaces`, and
`runtime -> major modules`.

## Microsoft NTVDM Research

Microsoft component work is not a formal runtime module or a committed backend.
It belongs under `docs/research/microsoft-ntvdm/` and, if required, owner-
approved one-off `tools/research/microsoft-ntvdm/` tools. It cannot create a
BOP framework, component loader, profile system, or dependency in the core
architecture before M9 T1/T2 research and an owner-approved M9 Go decision.

Historical NTVDM may be a coupled combination of guest DOS, machine emulation,
ROMs, BOP host services, console/redirection, and private Windows integration.
Research first establishes those boundaries; it does not pre-design an adapter.

## M1 Baseline Exception

M1 first establishes a runnable whole-NXVM baseline before subtractive
refactoring. Its imported source may temporarily live in
`src/nxvm-baseline/`, retaining upstream machine and platform coupling solely
to reproduce recorded baseline behavior. No new product feature belongs there.
M3 moves retained code into the final boundaries after the M2 design closes;
obsolete code is removed with focused evidence. This temporary area does not
relax provenance, MIT-authorization, copyright-notice, or platform-isolation
requirements.

The M1 snapshot banner is preserved unchanged so its source hashes remain
reproducible. The current M1 T2 developer artifact is the first approved
post-snapshot identity build and uses `0.4.015d.m1t2s1`; its one-line banner
deviation is recorded in its provenance. Later standalone artifacts follow the
same suffix rule.

The full baseline preserves the existing Linux platform implementation alongside
the Win32 implementation. M1 acceptance is the Windows GCC run; Linux is kept
as a source baseline for a future platform adapter. The long-term reusable core
is `machine + dos`; host Console, display, filesystem, and process behavior
remain platform adapters.

## Directory Plan

```text
src/
  nxvm-baseline/ app/ runtime/ machine/ dos/ platform/ adapters/ integration/
tests/
  machine/ dos/ platform/ adapters/ runtime/
docs/research/microsoft-ntvdm/
tools/research/microsoft-ntvdm/
tools/research/differential/
```

Directory README files define future ownership. This change does not copy, move,
or alter NXVM runtime code. NXVM assimilation begins only under a tracked
subtask with a provenance record and preserved license notices.

Historical sources provide orientation; tests provide validation; NXVM provides
the machine foundation; ntvdm64 provides the new integration.
