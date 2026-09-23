# T531 S27: Canonical Six-Package x86/Common/Lib Import

## Frozen source and scope

- Imported from SoftPC revision `f6dadddd2ce82e82d81094b2c510ebbfb427297b`.
- The SoftPC worktree contained unrelated pending product work, but all six
  imported trees were clean at that revision.
- The mechanically replaced trees are `src/lib`, `src/common`, `src/x86`,
  `test/lib`, `test/common`, and `test/x86`.

## Architecture and consumer adaptation

- Common remains neutral: it owns Machine, Session and UI only and has no x86
  include or target dependency.
- x86 is the selected architecture sibling.  Its Debug and xasm32 components
  use Common Machine's generic paused lease; NXVM selects those public
  contracts in App/VM, without restoring a Common Debug/xasm32 alias.
- `vm/machine/frame.c` is the sole NXVM adapter from Core display events to
  Common's copied Window/Console frame ABI.  It maps current Core text glyphs
  to the explicit CP437 character map and does not introduce a second frame
  owner.
- NXVM-only CMake and verification consumers were migrated to the x86 target.
  Historical specialized-gate ledgers now classify imported Common/x86 sources
  as their strict corpus targets instead of stale residual owners.

## Exact-corpus proof

`git diff --no-index --quiet` was run for each corresponding SoftPC/NXVM tree
after the final import.  All six comparisons returned equality:

```
src/lib=identical
src/common=identical
src/x86=identical
test/lib=identical
test/common=identical
test/x86=identical
```

`x86-verify` passed its manifest and component-boundary checks.  GCC 16 emits
two `format-truncation` diagnostics in the upstream-retained x86 Debug command
body; NXVM keeps that source byte-identical and limits the consumer-side
diagnostic downgrade to the affected x86 targets.

## Verification

- Full repository-only unit suite: **337 / 337 passed** (`ctest -L unit -j 8`,
  14.97 seconds wall time).
- External integration suite: **41 / 42 passed**.  The sole reproducible
  non-green row is `integration.vm-dos-keyboard-smoke`: it times out after
  30 seconds without its expected keyboard observation.  This is retained as
  an explicit existing product failure, not attributed to the S27 import.
- Session-readiness mutable-state inventory, documentation governance and the
  complete current specialized-gate aggregate passed after their imported
  Lib/Common/x86 paths were updated.
- `x86-verify` passed.
- `git diff --check` passed.

## Release artifacts

Both Release binaries are stripped (`-O3`, `-DNDEBUG`, linker debug stripping)
and are copied identically to `build/output` and `assets/sessions`:

| Architecture | SHA-256 |
| --- | --- |
| x64 | `05BDFD67625D7E6A96519BC2CC057A93B87311C15F4992B2E4C7E0A3B713AA36` |
| x86 | `267C9885A4AB0D3BF436A428E4A9F7E7FE3633B83D0C5F124FB994A1D95AF640` |

PE inspection confirms `i386:x86-64` and `i386`, respectively.

The owner accepted S27 on 2026-09-20. T531 remains intentionally open; S28
refreshes the same six canonical packages from the next frozen SoftPC revision.
