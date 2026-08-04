# ntvdm64

ntvdm64 is the successor repository for NXVM. It evolves the existing PC
emulator into a shared core with two first-class products:

```text
nxvm.exe      bootable whole-machine VM
ntvdm64.exe   non-bootable DOS application runner
```

The planned ntvdm64 product path is:

```text
ntvdm64 run [options] [<program> [args...]]
```

The planned `run` command has explicit display selection and host-drive
visibility controls. See [Runtime CLI Requirements](docs/requirements/ntvdm64-runtime.md).

## Formal Runtime

```text
core/{machine,platform,product}
  + vm/{machine,platform,product,profile}
  + vdm/{machine,platform,product,profile}
```

NXVM is the formal machine foundation and remains a supported product surface
through `nxvm.exe`. Its copyright holder authorizes NXVM code imported into this
repository under the root MIT License, with recorded source provenance and
preserved copyright notices. The owned DOS runtime is the default ntvdm64
compatibility path.

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
