# NXVM Product Tools

These scripts and ledgers serve only the NXVM product.  Shared tooling lives
under `tools/shared/`; do not add a MyNES route here.

## Fresh Product Build

Supply lawful BYOB assets outside this repository.  The current local default
is `O:/repos.hobby/nxvm-assets/profiles-nxvm`.  A product-only configuration
keeps MyNES targets out of the generated build graph:

```powershell
cmake -S . -B build/.ninja/gcc-x64-release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DREPOSITORY_BUILD_MYNES=OFF `
  -DNXVM_PROFILE_ASSETS_ROOT=O:/repos.hobby/nxvm-assets/profiles-nxvm `
  -DNXVM_PRODUCT_PROFILE=default-pc-at-80386-1440k-hdd
cmake --build build/.ninja/gcc-x64-release --target run-unit-tests --parallel 8
cmake --build build/.ninja/gcc-x64-release --target run-integration-tests --parallel 8
```

For x86, use `build/.ninja/gcc-x86-release` with the supported x86 compiler
and `-DPROJECT_ARTIFACT_ARCHITECTURE=x86`.  Select exactly one of the four
documented fixed profiles through `NXVM_PRODUCT_PROFILE`; CMake validates the
matching manifest, hashes and firmware slots before it builds.

Cache and temporary-output ownership follow
[Build Tree Hygiene](../../docs/rules/EXECUTION.md#build-tree-hygiene).
Enable optional ccache with `-DREPOSITORY_USE_CCACHE=ON` or a cache-enabled
preset; disabled presets clear the launcher in the same build tree.
The obsolete T196-only POSIX script has no current targets and is retired;
historical task records are not supported build instructions.

The four PC Apps deploy directly in `assets/my5160/`, `assets/my5170/`,
`assets/mydeskpro386/` and `assets/nxvm/`, without profile subdirectories.
Each pair remains beside its existing
owner-maintained `<app>.ini`. These PC Apps share this tool tree and task/version
sequence; they do not share private App composition sources.
Deployment never rewrites that INI. Runtime media paths in that INI
refer to the external `nxvm-assets/media-nxvm` archive; no protected asset is
copied into this repository.

## Script Classes

- `RunTestAggregate.ps1` is the bounded CTest runner used by the two product
  aggregate targets.
- `Verify-*.ps1` and the CPU timing/decoder ledgers are repository-only
  correctness checks.  Their names record their hardware or contract owner;
  they are not a second product launch route.
- `New-DosProbe.ps1` creates a temporary probe for an explicitly admitted
  integration investigation.  Do not version generated media or logs.
