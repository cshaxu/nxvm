# Coding Standard

Use the public [Coding Governance skill](https://github.com/cshaxu/skills/blob/main/coding-governance/SKILL.md)
for naming, readability, consistency, file cohesion, and duplicate-code
judgment. If a change affects ownership, dependencies, interfaces, or adapter
boundaries, also use the public
[Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).

The concrete source tree and naming map is
[design/CODING.md](../design/CODING.md).

## Source Discipline

- Use C11 and narrowly scoped, documented platform APIs. Keep repository source
  and code comments in English and prefer ASCII unless the domain requires
  another character set.
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
- Repair a repeated mechanism defect at its owning boundary; do not add a
  duplicate side path. Separate code needs a real semantic, layout, lifetime,
  or platform difference. Prefer a bounded owner-local helper to a broad
  framework when it expresses the complete shared contract.
- Treat new code as a cost with a named responsibility; prefer deletion,
  consolidation or an existing narrow boundary over wrappers and parallel paths.
- A replacement removes its obsolete implementation, forwarding layer and
  retired-path tests in the same task, unless evidence names a live caller and
  behavioral reason. Report the actual code-size change under the execution rule.

## Test Boundaries

- Tests prove an owned behavior or declared boundary. Test setup may exercise
  a same-module implementation when necessary, but must not create mirror
  state, a test-only public contract, or product-visible behavior.

Detailed historical vocabulary and migration inventory is retained in
[etc/history/m5/c-library-facade.md](../etc/history/m5/c-library-facade.md).
