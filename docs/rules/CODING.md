# Coding Standard

Use the public [Coding Governance skill](https://github.com/cshaxu/skills/blob/main/coding-governance/SKILL.md)
for naming, readability, consistency, file cohesion, and duplicate-code
judgment. If a change affects ownership, dependencies, interfaces, or adapter
boundaries, also use the public
[Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md).

The concrete source tree and naming map belongs to the selected product's
`docs/<product>/design/CODING.md`.

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
- Optimize for the simplest correct resulting design, not the smallest local
  diff. When a patch would preserve duplicate production paths, mirrored state,
  split authority, or an avoidable reverse dependency, replace and consolidate
  the affected mechanism at its owner instead of layering a workaround.
- Repair a repeated mechanism defect at its owning boundary; do not add a
  duplicate side path. Separate code needs a real semantic, layout, lifetime,
  failure-boundary, or platform difference. Prefer a bounded owner-local helper
  to a broad framework when it expresses the complete shared contract.
- Treat new code as a cost with a named responsibility. An abstraction is
  justified only when it removes duplicate implementation, state, or dependency
  complexity; a forwarding wrapper or parallel compatibility path is not a
  simplification.
- A replacement removes its obsolete implementation, forwarding layer and
  retired-path tests in the same task, unless evidence names a live caller,
  distinct behavior, and planned receiver. Report the actual code-size change
  and retained paths under the execution rule.

## Constant Meaning And Names

- A default or fallback value has `DEFAULT_` in its name. Do not name a default
  as though it were the actual storage dimension, active value or protocol size.
- A fixed layout, slot stride or resource dimension uses its direct domain
  name, for example `KVM_WINDOW_FONT_HEIGHT` or `X86_VIDEO_TEXT_GLYPH_ROWS`.
  Do not pair an unqualified default with a `MAX_` name for the real slot size.
  The default font height and the fixed glyph-slot height are distinct facts;
  an active frame height is a value, not a third compile-time owner.
- Use `CAPACITY` for buffer capacity, including any terminator or reserved slot.
  Use `INITIAL_` for an initial allocation that may grow. Neither is a default
  user value. `MAX_`/`MIN_` remain appropriate only for actual numeric or
  protocol bounds, not as substitutes for fixed layout dimensions or defaults.
  Platform/CRT vocabulary re-exported by Lib Types preserves its native meaning.
- Review definitions and all callers together, including tests and copied
  contracts. Rename without changing values, array layouts, validation or
  default behavior; remove the obsolete name rather than adding a synonym.

## Test Boundaries

- Tests prove an owned behavior or declared boundary. Test setup may exercise
  a same-module implementation when necessary, but must not create mirror
  state, a test-only public contract, or product-visible behavior.
- All test code is under the repository-root `test/` tree. Repository-only
  modules follow their `src/` owner; external-ROM, disk, firmware, media, or
  other owner-managed scenarios are only under the selected product's
  declared integration tree: existing Apps use `test/app-<product>/integration/`;
  MyRaidenII uses `test/myraidenii/integration/`. Neither is a shared unit corpus.

Product-specific migration records remain in the selected product's supporting
documentation.
