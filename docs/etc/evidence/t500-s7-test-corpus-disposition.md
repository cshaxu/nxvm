# T500 S7 Test Corpus Disposition

`t500-s2-test-inventory.json` freezes the input universe: 43 unregistered
executables and two EGA integration scenarios. This record is a disposition
ledger, not a second test registry. CMake remains the sole route registry.

## Method

Each candidate was rebuilt from `build/mingw-gcc-x64` and executed directly
where its declared input and host condition permit it. A five-second bounded
process check was used for no-input candidates; `TIMEOUT` is a failed test
contract, not a passing result. Source was compared with the current CMake
route and its asserted owner surface.

| Disposition | Targets | Basis and next batch |
| --- | --- | --- |
| Register as unit | `core-machine-plan-smoke`, `vm-model40-rom-layout-s14-smoke`, `vm-product-session-catalog-smoke`, `core-machine-instance-smoke`, `core-machine-explicit-time-s4-smoke`, six CPU ledger/manifest runners, `core-machine-debug-smoke`, `core-machine-presentation-smoke`, `vm-platform-request-bridge-smoke`, `core-machine-cpu-context-smoke`, `vm-fdc-authority-smoke`, `vm-debug-authority-smoke`, `core-machine-cpu-fpu-profile-smoke`, `core-machine-cpu-fpu-profile-closure-smoke` | Repository-only, distinct owner assertion, direct zero exit. S8 adds them to the sole CMake unit list. |
| Repair then register as unit | `core-machine-contract-smoke`, `vm-model-339-clock-contract-smoke`, `vm-session-profile-smoke`, `core-machine-lifecycle-smoke`, `core-machine-trace-smoke`, `core-machine-external-time-trace-s18-smoke`, `vm-cpu-probe-smoke`, `vm-cpu-stop-smoke` | Distinct owner surface, but nonzero exit or, for `vm-cpu-stop-smoke`, no termination. S8 repairs their assertion/runtime contract before registration. |
| Remove as redundant | `vm-cmos-authority-smoke`, `vm-fdd-authority-smoke`, `vm-hdd-authority-smoke`, `vm-bios-authority-smoke` | Each only constructs a session and checks that a local alias is non-null. Existing composition, FDD/HDD/firmware and lifecycle tests assert the actual owner contracts. S8 removes executable and source together. |
| Move and register as integration | `vm-product-default-profile-smoke`, `vm-execution-context-smoke`, `vm-full-pc-session-smoke`, `vm-byob-dos-boot-probe`, `vm-windows31-checkpoint`, `vm-windows31-setup-probe`, `vm-windows31-int13-trace-probe`, `vm-dos-fdisk-probe`, `vm-windows31-hdd-admission-probe` | The first three require FDD/HDD command inputs; the remaining six explicitly use owner-provided media/firmware. S9 relocates every source to `test/integration/` and registers scenarios only with declared BYOB arguments. |
| Conditional unit registration | `vm-platform-win32-startup-failure-smoke`, `vm-session-startup-failure-smoke`, `vm-platform-linux-run-handle-smoke` | Repository-only tests created only for their matching host/failure-stage configuration. S9 supplies one conditional CTest route when a target exists. |
| Split scenarios | `vm-ega-planar-dos-smoke`, `vm-rom-ega-int10-dos-smoke` | Independent external-media observables currently compiled from one macro-selected source. S9 creates two scenario sources in `test/integration/dos/`, with any shared fixture helper under the same owner directory. |

The retained external scenarios are exclusively under `test/integration/`. No
external file is a unit input, and no candidate is bulk-admitted because it
builds.

## Observed Results

The successful direct no-input executions were the 19 targets in the first
row. Current nonzero exits were `core-machine-contract-smoke`,
`vm-model-339-clock-contract-smoke`, `vm-session-profile-smoke`,
`core-machine-lifecycle-smoke`, `core-machine-trace-smoke`,
`core-machine-external-time-trace-s18-smoke`, and `vm-cpu-probe-smoke`.
`vm-cpu-stop-smoke` exceeded its five-second bound. Targets with command-line
media and host-only contracts were not misrepresented as no-input unit runs.
