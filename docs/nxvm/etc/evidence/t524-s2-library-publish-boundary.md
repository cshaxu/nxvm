# T524 S2 Library Publish Boundary

## Retained Source Authority

`src/lib/CMakeLists.txt` is the only library source inventory. Both the root
NXVM build and a standalone `cmake -S src/lib` build consume its selected
platform targets. `ux` is one root target; its contract/native target names are
aliases, not a cross-root dependency. The root build no longer maintains
`LIB_*` source lists or rebuilds a combined library corpus from a second
inventory.

`src/lib/MANIFEST.sha256` is revision-labelled and lists all 63 non-manifest
files beneath `src/lib`: public/private headers, Windows/Linux implementations,
the standalone CMake entry and its verifier. The manifest cannot list itself
without a self-referential hash; the verifier excludes only that one file.

## Drift Checks

`src/lib/verify_manifest.cmake` rejects malformed or duplicate lines, missing
files, stale paths, extra tree files and normalized-content hash mismatches.
`test/lib/library_manifest_fixture.cmake` proves the clean case plus missing,
extra and stale-content failures on a disposable build-tree copy. The former
partial `test/support/ux_library_manifest.cmake` verifier is removed.

`.github/workflows/lib.yml` configures the library directly on Windows and
Linux, invokes `lib-verify-manifest`, then builds the selected platform corpus.
It runs for every `src/lib/**` or workflow change.

## Verification

- Standalone Windows-selected configure/build: all five peer targets and
  `ux-native` build; `lib-verify-manifest` passes.
- Standalone CTest: `library.manifest` passes.
- Root fixture gate: clean, missing, extra and stale-hash cases pass.
- Root repository-only unit: 312/312 pass in 21.03 seconds with `-j 4`.
- Documentation governance and `git diff --check` pass.

No public C API or product/runtime behavior changes in this S.
