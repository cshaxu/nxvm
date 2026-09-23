# M6 T38 Product Layout Reconciliation

## Outcome

T38 completed the automated reconciliation for the owner-selected MyNes product
layout. Product code now lives below `src/app-mynes`: `core` owns the emulator
and `product` owns startup, configuration and commands. Product tests mirror
that ownership under `test/app-mynes/unit`, with cross-component tests under
`test/app-mynes/integration`. The current packet retains owner binary review
before task closure.

## Delivered Reconnection

- Root CMake traverses the moved product directories and gives their test trees
  unambiguous build-directory names.
- Product headers now use `product/...`; Core retains its stable `core/...`
  public include identity.
- The only packaged configuration, x64/x86 executables, and release manifest
  are in `assets/binary-mynes`.
- The release-manifest tool, README, active architecture/coding references,
  App/Core verification map, and documentation-governance fixture describe the
  new tree.
- T37's three unindexed evidence records were added to the supporting-document
  index, allowing the governance receiver to pass.

## Verification

- Fresh MinGW x64 configuration and build completed; 118 CTest cases passed.
- Fresh MinGW x86 configuration and build completed; 118 CTest cases passed.
- `tools/Verify-DocumentationGovernance.ps1` passed.
- `tools/Generate-ReleaseManifest.ps1` generated and verified the paired
  release manifest in `assets/binary-mynes`.

## Boundary

Lib and Common implementation files were not changed. Owner-local ROMs remain
ignored; the packaged `mynes.ini` remains the sole versioned configuration.
