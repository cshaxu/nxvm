# M5 T531 S2 Canonical Lib Refresh

## Source and exactness

The owner-controlled, clean SoftPC source is
`e140ec55bbc4cd40956948e57e58dc678e54f699`.  Its 93 tracked `src/lib/`
files replace NXVM's previous T530 `3cabea6a` corpus mechanically.  After
replacement, NXVM and SoftPC each have 93 Lib files and a per-file Git blob
comparison reports `different=0`.

The 11 changed files are `MANIFEST.sha256`, `README.md`, Console broker and
Win32 Console backend, UI-base input/public header, UI Console lifecycle,
Win32 Window component, its README and the type-layout verifier.  The source
review found no product, machine, profile, firmware, media, guest or native
handle exposure in the changed public surface.  It is project-owner-provided
MIT material already covered by the source-policy record; no other SoftPC path
was copied.

## Contract review and consumer sweep

- Broker creation now quiesces a failed native activation before disposal;
  backend destruction no longer pretends to stop a reader.
- Win32 Console establishes its text surface after palette application and
  treats clipped writes as failures; its cached frame is invalidated first.
- Window rendering reports failed native paint/invalidation/cursor creation
  through the existing component failure path.
- UTF-16 packet normalization is made file-private; the direct whole-repository
  search found no non-Lib caller.
- UI Console disposal relies on the worker's already-drained callback
  lifecycle. Common UI is the only non-Lib Console/UI caller; its normal
  component destroy path is covered by the complete unit suite.

No NXVM Common, VM, Core, profile, YAML, test or CMake consumer adaptation was
needed. The source changes 105 Lib lines: 83 added, 22 removed, net +61
(Git numstat including manifest/README). The retained production route is one
exact shared Library corpus.

## Verification

- SoftPC worktree clean and source revision verified.
- 93/93 NXVM-to-SoftPC Lib hashes equal; `different=0`.
- `verify_manifest.cmake` and `verify_component_dependencies.cmake` pass.
- `cmake --build --preset unit-tests-gcc` passes: 299/299 repository-only unit
  tests; real time 19.96 seconds. Its Lib/Common checks and documentation gate
  pass.

External integration and release-artifact rebuilding are not run for this S:
no product behavior, asset, artifact identity or executable source changed.
They remain required when a later runnable S changes a product path.
