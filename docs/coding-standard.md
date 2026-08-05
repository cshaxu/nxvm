# Coding Standard

Use the public [Coding Governance skill](https://github.com/cshaxu/skills/blob/main/coding-governance/SKILL.md)
for naming, readability, consistency, file cohesion, and duplicate-code
judgment. If a change affects ownership, dependencies, interfaces, or adapter
boundaries, also use the public
[Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).

This document records ntvdm64-specific source rules:

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
