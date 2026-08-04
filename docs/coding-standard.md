# Coding Standard

This is the concise source standard. [`../RULES.md`](../RULES.md) remains the
implementation and review checklist; if detail is needed, use it with
[Module Layout](architecture/module-layout.md) and
[Source Policy](source-policy.md).

- Write C11 with the `src/type.h` vocabulary: `C_*` scalar types, `STD_*`
  standard-library facade, and platform-prefixed system types.
- Put code in its real owner. `core` never depends on `vm` or `vdm`; sibling
  modules do not depend on each other; product-root composition is the only
  integration point.
- Expose cross-module contracts as `*_interface.h`; install implementations as
  `*_provider`. Public symbols carry their owner prefix.
- Keep headers beside implementations. Prefer a cohesive owner over forwarding
  facades, permanent aliases, or interface layers with no boundary purpose.
- Preserve one executor and one state owner. Production paths may not select a
  machine/session through globals, TLS, singletons, or implicit current state.
- Keep host integration out of guest state. Platform code reports through its
  contract; machine mutation occurs at the product command boundary.
- Make changes narrow and test the affected runnable path. Do not rewrite or
  copy NXVM code merely to fill a new layout; preserve provenance and use
  `git mv` for structural relocation.
- Use `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` only with a ledger entry and
  a bounded future admission path.
