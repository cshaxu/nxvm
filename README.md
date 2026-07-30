# ntvdm64

ntvdm64 is a DOS compatibility runtime evolved from NXVM, combining reusable
PC emulation, an independently implemented DOS environment, and non-invasive
Windows host integration.

The primary product path is:

```text
ntvdm64 run [options] [<program> [args...]]
```

The planned `run` command has explicit display selection and host-drive
visibility controls. See [Runtime CLI Requirements](docs/requirements/runtime-cli.md).

## Formal Runtime

```text
NXVM machine foundation
  + independent DOS runtime
  + non-invasive Windows platform integration
  + adapters, runtime composition, and CLI
```

NXVM is the formal machine foundation. Eligible LGPL-licensed CPU, debugger,
hardware, BIOS, and device code can be imported with recorded provenance and
preserved notices. The owned DOS runtime is the default compatibility path.

The default release is buildable and usable without Microsoft binaries, WineVDM,
host injection, loader replacement, registry changes, or administrator rights.

## Long-Term Research

- invasive Windows integration;
- historical Microsoft NTVDM component feasibility; and
- Win16 feasibility, including external WineVDM evaluation.

These are not committed backends or present product features. Microsoft
components are never redistributed. Research cannot become an implicit runtime
dependency or delay the distributable DOS product.

## Current State

M0 established governance, the probe laboratory, source/import policy, and the
module plan. No NXVM runtime code has been imported and no DOS program runs
yet. The roadmap begins with NXVM machine validation, then minimal DOS, then
the non-invasive CLI product path.

Read [AGENTS.md](AGENTS.md) before changing the repository. Canonical guidance
is in [architecture](docs/architecture.md), [roadmap](docs/roadmap.md), and the
[project constitution](docs/project-constitution.md).
