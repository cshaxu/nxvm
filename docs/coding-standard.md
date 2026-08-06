# Coding Standard

Use the public [Coding Governance skill](https://github.com/cshaxu/skills/blob/main/coding-governance/SKILL.md)
for naming, readability, consistency, file cohesion, and duplicate-code
judgment. If a change affects ownership, dependencies, interfaces, or adapter
boundaries, also use the public
[Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).

This document records ntvdm64-specific source rules:

## Source Layout

The only permitted source roots are `core`, `vm`, `mantle`, `dos`, and `vdm`.

- `core/utils` is the lowest neutral utility/callback layer and depends only on
  `type-facade`. `core/{machine,platform,product}` contains behavior shared by
  VM and VDM. A concrete host implementation belongs under
  `core/platform/<host>` only when both products can reuse it without product
  policy.
- `vm/{machine,platform,product,profile}` contains bootable whole-machine
  behavior, the retained VM Console/debugger experience, and machine profiles.
- `mantle/{machine,platform,product}` contains only policy-free VDM composition
  mechanism shared by NXVDM and an admitted external implementation.
- `dos/{machine,platform,product,profile}` contains the owned DOS backend and
  depends on no other product component.
- `vdm/{machine,platform,product,profile}` contains NXVDM application-runner
  behavior over mantle and dos.

Public headers live beside their implementations. The complete ownership and
dependency graph remains [Module Layout](architecture/module-layout.md).

- [C-Library Facade](architecture/c-library-facade.md) is the sole detailed
  authority for `src/type.h`, C/standard-library vocabulary, platform type
  exposure, and direct ISO C header/call restrictions.
- The exact component dependency graph, public symbol prefixes,
  and composition ownership are defined by
  [Module Layout](architecture/module-layout.md) and
  [Contracts](architecture/contracts.md).
- A cross-module contract is `*_interface.h`; injected implementations are
  `*_provider`. Public symbols use their source-owner path.
- Prefer the smallest existing boundary. Do not add getters, setters,
  snapshots, adapters, or test-only contracts solely to hide a pointer that
  the owning production implementation already exposes and may legitimately
  use. A test may include and directly exercise that implementation boundary
  when it tests the same module, uses no mirror state or alternate runtime
  route, and remains readable as a real behavior test. Promote a capability to
  `*_interface.h` only when an independent production owner needs the
  capability; test convenience is not sufficient reason.
- Test helpers are justified only when they remove repeated setup, make a
  behavior assertion clearer, or protect a genuinely separate production
  boundary. They must not become a second facade over ordinary module state.
- Headers stay beside implementations. Do not introduce module-local scalar
  aliases or a second C-library facade.
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
