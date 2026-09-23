# M5 T512 S6 - 80186 final audit

## Coverage and authority

The frozen 80186 List-2 universe is the 279 base forms plus 337 legal context
forms emitted by the 80186 timing manifest.  The Intel 1985 80186/80188 User's
Manual is normative.  A direct exact number or formula is Manual-L3; a manual
range is L2 even where this implementation selects its documented midpoint.

## Corrections

`ENTER` now preserves the 80186 lexical-level byte (0--255); only 80286 and
80386 apply their separate five-bit rule.  The Core instruction recorder no
longer faults a valid level-255 frame merely because its debugger observation
buffer was full.  The timing selector applies the Table 2-9 formula
`22 + 16 * (level - 1)` to the full 80186 byte.  Targeted checks cover levels
33 (534 clocks) and 255 (4086 clocks).

The final sweep also corrected stale exact-Manual-L3 test expectations for
80186 direct near and far jumps (14 clocks), and changed the 8088 retirement
test from an obsolete unallocated assertion to its actual L2 dynamic-arithmetic
provenance.  It does not change 8088 timing behavior.

## Verification

- `core-machine-80186-instruction-timing-ledger-smoke.exe`: pass.
- `core-machine-80186-timing-manifest-runner.exe`: 616 observations emitted.
- `Verify-80186TimingResults.ps1` over the freshly generated result: 616/616
  conforming keys.
- `Verify-80186TimingManifest.ps1` and `Verify-80186DecoderLedger.ps1`: pass.
- complete repository-only `ctest -L unit -j 8`: no `Test Failed` entry in the
  current CTest log; the two corrected regression tests pass.

## Owner and residual classification

Core remains the sole owner of 80186 decode, execution, retirement and timing.
No VM/profile CPU behavior was added.  The 26 manual-range rows remain L2 by
definition; the 253 exact/formula rows remain Manual-L3.  No new L1 timing row
was found in this S.
