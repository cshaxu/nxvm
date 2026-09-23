# T447 S7 VM contract boundary

## Result

VM session, media, profile, product, and composition consumers no longer share
their owners' mutable layouts.  FDD/HDD, session, default-PC/AT profile, and
Model-40 D4 state are opaque outside their owner; callers receive operations,
copied catalog selections, or bounded media observations.  Keyboard scan-set
selection is a bounded Core enum, and the former private Core firmware-header
dependencies now belong to the VM firmware owner.

Model-40 has one public BYOB creation path.  Test firmware is copied into that
path through a test-only file fixture; the retired direct constructor has no
remaining declaration or caller.  Immutable ROM bus writes retain Core's
documented accepted-and-discarded behavior, and the layout smoke proves the
backing remains unchanged.

The Console provider contract belongs to product; its concrete binding is in
composition, the sole VM layer permitted to join product, profile, machine,
and platform capabilities.  The dependency DAG has no migration allowlist
edges.  Parser state remains solely in the opaque catalog and Console handles.

## Removed routes

- Public FDD/HDD, session, default-PC/AT profile, and Model-40 D4 layouts.
- The duplicate `vm_session_create_model40_private` construction route.
- Product-to-composition provider dependency and the catalog test compatibility
  macro.
- Private KBC scan-set constants and five stale private Core include routes.

## Verification

- `cmake --build build/mingw-gcc-x64 -j 4` completed.
- `verify-current-specialized-gates` completed all 76 aggregate static gates,
  including T447 S1--S6, dependency-DAG, T345, documentation governance, and
  the updated worktree-aware vocabulary audit.
- Focused Model-40 suite: 20/20 configured tests passed; independent
  `vm-model40-rom-layout-s14-smoke` passed.
- Full `ctest --test-dir build/mingw-gcc-x64 --output-on-failure` completed:
  292 `Test Passed`, 0 `Test Failed` in `Testing/Temporary/LastTest.log`.
- Current `vm-0-5-0447` artifact SHA-256:
  `CAA87452E18E90C5E72114D8C8B6C04FD830920BB45DA160CA6562787AA3B920`.

## Minimalism accounting

The production boundary uses owner-local private headers and existing
capability operations; it adds no compatibility alias, forwarding path,
generic profile framework, or test-only production API.  Excluding
documentation, the staged source/test/CMake change is 1,039 added and 735
removed lines (net +304).  The addition is the owner-local private-layout
files, a copied-input test fixture, bounded media observations, and corrected
static guards; the removed layouts, constructor, constants, includes, and
aliases are not retained in a second path.
