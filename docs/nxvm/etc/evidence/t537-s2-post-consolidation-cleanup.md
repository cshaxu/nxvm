# M5 T537 S2 Post-Consolidation Cleanup

## Scope

This continuation removes the concrete coupling found after the repository
consolidation. It changes no emulator behavior, external asset route, product
task state, or MyNES source policy.

## Result

- Root CMake owns C11/toolchain validation, architecture identity and neutral
  Lib/Common/x86 traversal only. `cmake/nxvm/NxvmProduct.cmake` now owns the
  NXVM graph; `cmake/mynes/MyNesProduct.cmake` owns the MyNES graph.
- MyNES presets set `REPOSITORY_BUILD_NXVM=OFF`; they no longer carry an NXVM
  profile value. NXVM keeps its own selected profile contract.
- MyNES has separate unit and integration binary aggregates. The unit runner
  no longer requires integration executables.
- NXVM scripts resolve their namespaced `tools/nxvm` location and default root
  correctly after the product-tool move. The aggregate self-test and CPU
  catalog export both passed.
- `docs/rules/` is the sole shared documentation corpus. NXVM and MyNES retain
  independent guides, states, queues, proposals, history and evidence.
- The root README equals pre-merge commit `36925a4b`; owner direction freezes
  it from further modification.

## Verification

- `Verify-DocumentationGovernance.ps1 -Product nxvm`: pass.
- `Verify-DocumentationGovernance.ps1 -Product mynes`: pass.
- `VerifyTestAggregate.ps1 -RepositoryRoot .`: pass.
- `Export-CpuTimingManifestCatalog.ps1` to two temporary files: pass.
- NXVM CMake configure: pass.
- A fresh MyNES-only configure and a Ninja target-help query both stalled in
  this host's compiler/Ninja ABI-detection path before a product target ran.
  The owned processes and temporary build tree were removed. This is recorded
  as a host limitation, not a passing build/test claim.
