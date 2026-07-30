# ntvdm64

ntvdm64 is a DOS and Win16 compatibility runtime evolved from NXVM. It combines
reusable PC emulation, an independently developed DOS layer, Windows platform
adapters, and optional compatibility with historical Microsoft NTVDM guest
components.

The primary user path will be non-invasive command-line execution:

```text
ntvdm run <program> [args]
```

The default backend is project-owned and distributable without proprietary
guest components. NXVM is the machine foundation: its eligible LGPL-licensed
CPU, debugger, hardware, BIOS, and device code can be imported with recorded
provenance and preserved notices. DOS compatibility, platform integration, and
runtime composition are maintained in ntvdm64's own module structure.

## Architecture

```text
NXVM machine foundation
  + owned DOS module
  + non-invasive Windows platform integration
  + optional Microsoft NTVDM guest compatibility
  + isolated invasive integration research
```

The module boundaries are `machine`, `dos`, `platform`, `adapters`, `runtime`,
and `app`. `microsoft` is a future BYOB research boundary; `integration` is
research-only and excluded from normal builds and releases.

## Product Rules

- The owned DOS backend is the default and highest-priority path.
- Microsoft guest files are optional, user-supplied, profile/hash validated,
  and never redistributed by this project.
- OpenNT and NTVDMx64 are historical research sources, not copied code.
- Invasive Windows integration is limited to research, TODOs, and approved
  prototypes. No global injection, loader replacement, or automatic system
  changes are part of the default product.
- Win16 remains research-only. WineVDM may be evaluated as an external backend.

## Current State

M0 established governance, source/license boundaries, the CMake probe
laboratory, and directory ownership. No NXVM runtime code has been imported and
no DOS program can run yet.

Read [AGENTS.md](AGENTS.md) before changing the repository. Canonical guidance
is in [architecture](docs/architecture.md), [roadmap](docs/roadmap.md), and the
[project constitution](docs/project-constitution.md).
