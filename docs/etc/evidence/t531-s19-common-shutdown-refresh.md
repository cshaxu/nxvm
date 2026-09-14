# M5 T531 S19 - Canonical Common Shutdown Refresh

## Result

The owner-authorized SoftPC revision
`433f57eb8d57ee4dde0ee6a02eb4c19932e51cd0` supplies the exact NXVM
`src/common` and `test/common` corpus.  Both post-import tree comparisons
returned zero differences.

Common now exposes `common_machine_shutdown()`: it permanently joins the
worker and all callbacks without freeing the Common object.  NXVM App uses the
same lifecycle in the only valid order:

```
Common shutdown -> UI destroy -> session destroy -> Common destroy
-> revoke VM binding -> VM destroy
```

The imported Common smoke blocks the final callback and proves shutdown cannot
return before that callback completes.  It also proves repeated shutdown is
safe and that start, reset and removable-media requests reject afterwards.

## Sweep and verification

- The tracked production/test/build sweep for `common_machine_destroy`,
  `common_machine_create` and `common_machine_shutdown` found App composition
  as the only NXVM production Common owner.  Test fixtures own local Common
  lifetimes and destroy their callback contexts after Common destruction; no
  production callback target is released before shutdown.
- `ctest --test-dir build/mingw-gcc-x64-release -L unit -j 8
  --output-on-failure`: **325/325 passed** in 22.71 seconds.
- Documentation governance and `git diff --check` passed.
- Stripped optimized artifacts were directly linked from the generated CMake
  object/library lists after the host Ninja wrapper stalled.  Architecture
  verification passed for both copies of each executable.
  - x64: `7EC0FBD1023F3041DE1F7ED0FD3A66FB23157B614014BD7C6F8692BB53FAF09C`
  - x86: `04D180B982F8783EECCFD0242334C670F458CBE26F500F4360F868315A854BA4`

## Size and boundary

Tracked code/test changes add 81 and remove 4 lines (net +77), principally the
canonical blocked-callback shutdown proof.  The retained production path is
one Common worker and one App composition cleanup path; no NXVM-specific
Common variant or compatibility route remains.

## Acceptance

Coordinator actual-diff review of P1 confirms the imported corpus is exact,
the only NXVM adaptation is App's dependency-safe shutdown ordering, and the
test adds the required lifetime proof rather than a parallel lifecycle path.
S19 is accepted. T531 remains open for its separately admitted package audit.
