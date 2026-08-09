# Coding Standard

Use the public [Coding Governance skill](https://github.com/cshaxu/skills/blob/main/coding-governance/SKILL.md)
for naming, readability, consistency, file cohesion, and duplicate-code
judgment. If a change affects ownership, dependencies, interfaces, or adapter
boundaries, also use the public
[Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).

The concrete source tree and naming map is
[design/CODING.md](../design/CODING.md).

- Use clear, searchable names with one meaning per symbol. Keep a cohesive
  file focused on one owner and responsibility; split only when a real
  subsystem boundary exists.
- Use one project type and C-library vocabulary. Do not introduce local scalar
  aliases, a second type facade, or platform SDK types into platform-neutral
  public code. Unsafe input flushing is prohibited; use an explicit capability.
- A public header exposes only the minimum durable capability. Keep mutable
  layouts and implementation helpers private; do not manufacture getters,
  setters, snapshots, wrappers, or facades merely to hide a pointer.
- Prefer an existing clear boundary over a duplicate abstraction. A helper,
  adapter, or compatibility layer needs a named responsibility and cannot
  merely forward the same object.
- Tests prove an owned behavior or declared boundary. Test setup may exercise
  a same-module implementation when necessary, but must not create mirror
  state, a test-only public contract, or product-visible behavior.

Detailed historical vocabulary and migration inventory is retained in
[etc/architecture-notes/c-library-facade.md](../etc/architecture-notes/c-library-facade.md).
