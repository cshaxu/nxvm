# Coding Standard

Use the public [Coding Governance skill](https://github.com/cshaxu/skills/blob/main/coding-governance/SKILL.md)
for naming, readability, consistency, file cohesion, and duplicate-code
judgment. If a change affects ownership, dependencies, interfaces, or adapter
boundaries, also use the public
[Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).

This document records ntvdm64-specific source rules:

## Source Layout

The only source roots are `core`, `vm`, and `vdm`.

- `core/{machine,platform,product}` contains behavior shared by VM and VDM.
  A concrete host implementation belongs under `core/platform/<host>` only
  when both products can reuse it without product policy.
- `vm/{machine,platform,product,profile}` contains bootable whole-machine
  behavior, the retained VM Console/debugger experience, and machine profiles.
- `vdm/{machine,platform,product,profile}` contains DOS application-runner
  behavior and DOS execution profiles.

Public headers live beside their implementations. The complete ownership and
dependency graph remains [Module Layout](architecture/module-layout.md).

- Write C11 with the `src/type.h` vocabulary: `C_*` scalar types, `STD_*`
  standard-library facade, and platform-prefixed system types.
- The exact `core`, `vm`, and `vdm` dependency graph, public symbol prefixes,
  and composition ownership are defined by
  [Module Layout](architecture/module-layout.md) and
  [Contracts](architecture/contracts.md).
- A cross-module contract is `*_interface.h`; injected implementations are
  `*_provider`. Public symbols use their source-owner path.
- Headers stay beside implementations. `src/type.*` is the sole common
  type/C-library facade; do not introduce module-local scalar aliases or raw
  ISO C calls outside it.
- Production paths may not select a machine/session through globals, TLS,
  singletons, or implicit current state. Platform code reports through its
  contract; product composition performs machine mutation at its command
  boundary.
- Structural NXVM relocation preserves provenance: use `git mv`, repair direct
  dependencies, verify, then move the next bounded source file.
- Use `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` only with a ledger entry and
  bounded future admission path.
- Defect and compatibility fixes follow the mandatory similar-issue sweep in
  [Execution Policy](planning/execution-policy.md); do not apply a local patch
  while leaving equivalent production paths unclassified.
