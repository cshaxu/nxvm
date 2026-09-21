# T531 S28: Canonical Six-Package Refresh

## Frozen source and scope

- Imported from committed SoftPC revision
  `2b17749a12c1132d9c9c65754008befbcb764546`.
- SoftPC's six imported trees were clean at that revision; unrelated sibling
  product work was not imported.
- The mechanically replaced trees are `src/lib`, `src/common`, `src/x86`,
  `test/lib`, `test/common`, and `test/x86`.

## Boundary result

- The six trees were replaced mechanically and remain byte-for-byte identical
  to the frozen source. NXVM contains no local patch or compatibility branch
  in them.
- The retained architecture is unchanged: Lib is neutral host service, Common
  owns machine/session/UI product runtime mechanisms without x86 dependency,
  and x86 is the selected Debug/xasm32 sibling.
- NXVM consumer composition stayed on the existing public Lib/Common/x86
  contracts; no second frontend, source alias, or asset/session-YAML path was
  introduced.

## Exact-corpus and verification proof

`git diff --no-index --quiet` returned equality for every frozen pair:

```
src/lib=identical
src/common=identical
src/x86=identical
test/lib=identical
test/common=identical
test/x86=identical
```

- `x86-verify` passed its manifest and boundary checks.
- The complete specialized-gate aggregate passed.
- Full repository-only unit suite: **338 / 338 passed** (`ctest -L unit -j 8`,
  21.62 seconds wall time). The refreshed x86 corpus adds its bounds smoke.
- External integration: **41 / 42 passed**. The one non-green row is the
  pre-existing `integration.vm-dos-keyboard-smoke`; it is recorded without
  attribution to this import.
- `git diff --check` passed.

## Release artifacts

Both Release binaries are rebuilt, optimized and stripped, and copied
identically to `build/output` and `assets/sessions`:

| Architecture | SHA-256 |
| --- | --- |
| x64 | `235C9F409E064808C691769B1BFDFDD6C84BEB105D40CBD044827810B1C71B27` |
| x86 | `CA4C4A91A62C318F80DCDA86AD3781C2D0A072FD04AAA82C36AE7061FF2E3BF8` |

PE inspection confirms `i386:x86-64` and `i386`, respectively.

The owner accepted S28 and closed T531 on 2026-09-20. The sole non-green
integration row remains explicitly transferred to `TODO(High)`.
