# T537 S1 Multi-App Consolidation Evidence

## Scope

The owner approved merging the NXVM and MyNES product trees under this existing
repository root. This record covers only topology, build entry points, tooling,
documentation, notices, and retained executable artifacts. It introduces no
emulator behavior or protected asset input.

## Result

- Root governance is in `docs/rules/`; NXVM and MyNES each own a complete
  `docs/<product>/` design, state, proposal, history, and supporting-record tree.
- Neutral source/test components remain at `src/{lib,common,x86}` and `test/`.
  Product code and tests live at `src/app-<product>` and `test/app-<product>`.
- NXVM CMake support is under `cmake/nxvm`; the full MyNES CMake traversal is
  represented by `cmake/mynes/MyNesProduct.cmake`. The upstream CMake inventory
  contains 14 files, all present in the merged topology.
- Root CMake configures shared components and exposes MyNES executable, unit,
  integration, and aggregate test targets without duplicating its source CMake.
- Product tools are under `tools/nxvm` and `tools/mynes`; the shared document
  topology check is the sole `tools/shared/Verify-DocumentationGovernance.ps1`.
- The root `LICENSE` is the sole MIT text. `THIRD_PARTY_NOTICES.md` records
  both product provenance. `assets/binary-nxvm` and `assets/binary-mynes` are
  versioned; MyNES ROM test inputs remain ignored.

## Verification

- `tools/shared/Verify-DocumentationGovernance.ps1 -RepositoryRoot . -Product nxvm` — pass.
- `tools/shared/Verify-DocumentationGovernance.ps1 -RepositoryRoot . -Product mynes` — pass.
- `cmake --preset mingw-gcc-x64-release` — configure/generate pass.
- `ninja -C build/mingw-gcc-x64-release -n mynes-0-1-0011` — MyNES target graph
  resolves through 67 compile/link steps.
- `ninja -C build/mingw-gcc-x64-release -n run-mynes-unit-tests` — MyNES unit
  aggregate resolves through 172 compile/link/test steps; 53 MyNES CTest rows
  are registered.
- `git diff --cached --check` — pass; staged-content scan found no `.nes` or
  `.rom` files.

## Build-Host Note

Fresh native Ninja execution in this workspace stalls before dispatching a
compiler process, including a clean temporary CMake probe. The existing
versioned MyNES x64/x86 artifacts are retained as owner-provided binaries; this
task does not claim a new binary build. The CMake configure and complete dry-run
graphs prove the merge wiring, while actual native build execution remains a
host/toolchain issue to resolve before a future artifact refresh.
