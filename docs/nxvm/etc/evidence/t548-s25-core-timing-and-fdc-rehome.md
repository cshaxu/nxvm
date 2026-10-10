# T548 S25 Core Timing, Decoder And FDC Test Rehome

## Scope And Decision

The former Default-App board directory contained four CPU timing-manifest
runners and one FDC boundary negative script.  Three CPU decoder inventory
runners were already physically in `test/core`, but their targets were still
declared by the NXVM App graph and linked through the Model 40 D4 aggregate.
None of these assertions selects a fixed PC profile.

They are consequently owned by Core:

- timing-manifest runners now live under `test/core/x86`;
- generated timing catalog/result and decoder-ledger CTest consumers are
  declared by `test/core/x86_tests.cmake`;
- decoder-inventory runners link `core-chip-cpu` directly;
- 80286/80386 timing runners link `core-board-base` and `core-x86`, their
  actual fixture owners;
- the FDC boundary negative script lives in `test/core` and is labelled
  `unit;core`.

The root unit-target list remains a registration index only.  It retains the
Core target names so the repository-wide exact registration gate can account
for every CTest route; it is not an App ownership declaration.

## Assertion Preservation

The timing runners continue to generate the same build-tree result documents
and use the same five manual timing manifests.  The existing 8086, 8088 and
decoder-ledger consumers retain their original predicates.  The FDC negative
script retains its product-boundary assertion while its execution owner moves
to the Core package.  No machine composition, timing assertion, firmware,
media or production source changed.

## Verification

Fresh `t548-s25` Ninja graphs verified both widths:

- twelve exact Core routes pass on x64 and x86: FDC negative, five timing
  runners, three result/ledger consumers and three decoder inventory runners;
- `core.manifest` and `core.test-manifest` pass on x64 and x86;
- `verify-fixture-shapes` passes on x64 and x86 with 135 classified direct
  constructors;
- `verify-unit-test-registration` passes on x64 and x86 with 335 registered
  unit routes;
- a direct `test/core/MANIFEST.sha256` hash, missing-file and stale-entry
  scan reports zero mismatches; and `git diff --check` is clean.

No desktop, external-media, integration or full-suite qualification is claimed
by this test-only receiver.
