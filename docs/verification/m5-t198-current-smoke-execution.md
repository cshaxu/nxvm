# M5 T198 Current Smoke Execution Verification

## Closed Scope

`current-gates-gcc` no longer treats smoke executable linkage as runtime
coverage. `run-current-smokes` builds the selected binaries, then invokes
CTest with the `current-gate` label and `--output-on-failure`. Static
source/inventory checks remain separate build targets.

## Evidence

- Fresh GCC configuration discovered 40 labeled `current.*` tests.
- The current-gate preset executed all 40: `100% tests passed, 0 tests failed`.
  The executed set includes KBC, VADP text, platform run-handle, DOS prompt,
  DOS keyboard, DOS MEM, Console lifecycle, and VDM minimal smoke coverage.
- DOS fixture tests receive the owner-provided sibling `fdd.img` and `hdd.img`
  through `PROJECT_FDD_SMOKE_IMAGE` and `PROJECT_HDD_SMOKE_IMAGE` cache
  variables. The media remains untracked.
- The same preset then passed its separate static dependency, ownership,
  lifecycle, Linux source-contract, and current-artifact checks.
- `nxvm_0_5_0198.exe` printed its matching banner, accepted piped `EXIT`, and
  has SHA-256 `6B40CA27D30B5C216F68A195E7CC9560151A56891672F3D6A50AA9CA1B1637AB`.
