# ntvdm64

ntvdm64 is the successor repository for NXVM. It evolves the existing PC
emulator into a modular runtime with two executable products and three planned
shared libraries:

```text
nxvm.exe      bootable whole-machine VM
nxvdm.exe     non-invasive DOS application runner
```

The planned NXVDM product path is:

```text
nxvdm run [options] [<program> [args...]]
```

The planned `run` command has explicit display selection and host-drive
visibility controls. See [Runtime CLI Requirements](docs/requirements/nxvdm-runtime.md).

## Target Component Topology

```text
core/{utils,machine,platform,product}   shared foundation; future core.dll
vm/{machine,platform,product,profile}   nxvm.exe composition and product
mantle/{machine,platform,product}        future shared VDM composition; future mantle.dll
dos/{machine,platform,product,profile}   independent owned DOS backend; future dos.dll
vdm/{machine,platform,product,profile}   nxvdm.exe product shell over mantle + dos
```

NXVM is the formal machine foundation and remains a supported product surface
through `nxvm.exe`. Its copyright holder authorizes NXVM code imported into this
repository under the root MIT License, with recorded source provenance and
preserved copyright notices. The owned DOS runtime is the default NXVDM
compatibility path. `core.dll`, `mantle.dll`, and `dos.dll` are medium-term
build targets, not current independent artifacts.

The current tracked source tree contains `core/`, `vm/`, and a non-runnable
`vdm/` skeleton only. `mantle/` and `dos/` are planned roots and do not yet
exist; the skeleton is not `nxvdm.exe` or an owned DOS backend. The authoritative
current-versus-target inventory is [Module Layout](docs/architecture/module-layout.md).

The default products are buildable without Microsoft binaries, WineVDM, host
injection, loader replacement, registry changes, or administrator rights.

## Long-Term Research

- invasive Windows integration;
- historical Microsoft NTVDM component feasibility; and
- Win16 feasibility, including external WineVDM evaluation.

These are not committed backends or present product features. Microsoft
components are never redistributed. Research cannot become an implicit runtime
dependency or delay the distributable DOS product.

## Current State

M0 through M4 are complete. M1 imported and preserved the NXVM baseline under
the root MIT authorization and proved a GCC-built whole-machine baseline. M2
defined the shared-core architecture; M3 established the shared-core source
shape and VDM profile tests; M4 completed the firmware, profile, CPU, and
retained NXVM Console design contracts. M5 is the active implementation and
closure milestone; its current state, active subtask, and verification evidence
are maintained in [Project Status](docs/planning/status.md).

Read [AGENTS.md](AGENTS.md) before changing the repository. Canonical guidance
is in the [documentation guide](docs/README.md),
[architecture](docs/architecture/overview.md),
[roadmap](docs/planning/roadmap.md), and the
[project constitution](docs/project-constitution.md).
