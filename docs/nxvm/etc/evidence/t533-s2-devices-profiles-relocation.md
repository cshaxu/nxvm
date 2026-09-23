# T533 S2 Devices And Profiles Relocation

S2 consumes the S1 path-only relocation batch.  It makes these live owner
renames, without changing a public target name, C interface or runtime path:

- `src/core/core` is now `src/core/devices`.
- `src/core/profile` is now `src/core/profiles`.
- Their repository-only mirrors are now `test/core/devices` and
  `test/core/profiles`.

CMake source lists, direct includes, source-shape verifiers, PowerShell
verification scripts and the dependency allow-list now name the moved owners.
The six Machine-level tests that deliberately share the CPU fixture now include
`../devices/support/core_machine_cpu_fixture.h`; that completes the single
relative-fixture path class.  No Machine composition, App/catalog/YAML route,
asset read, firmware-provider decision, test assertion or target identity moved
in this S; those are the S3/S4 receivers frozen in the S1 ledger.

## Sweep And Verification

- Literal sweeps over tracked production source, tests, CMake and tools find
  zero live occurrences of `src/core/core/`, `src/core/profile/`,
  `test/core/core/`, `test/core/profile/`, `"core/core/` or `"core/profile/`.
  Historical documentation remains an intentional record of the old baseline.
- A fresh reconfigure of `build/t533-s1-x64` resolves the moved source lists.
  The complete repository-only unit suite passes 336/336 after the fixture
  correction; no external asset or integration input is involved.
- `git diff --check` and documentation governance pass.  Actual-change review
  confirms a path-only relocation plus direct path repairs, with no new source
  owner or compatibility alias.
