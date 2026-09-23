# M3 T19: Final Reconciliation And Closure

T19 closes M3 after reconciling its remediation ledger, presentation outcomes,
current source boundaries and usable developer-artifact delivery.

## S1 Audit And Delivery Repair

The audit found one product-owned delivery defect: the versioned artifacts
required by the execution rules were not being copied to `build/output/` with
the `mynes.ini` file that the executable must read beside itself. The source
build tree had a valid configuration copy, but the preserved developer artifact
directory retained stale earlier executables and was therefore not independently
usable.

`src/app/CMakeLists.txt` now derives x64 or x86 from the configured pointer
width and post-build copies the stripped current executable as
`mynes_0_1_0011_<arch>.exe`, plus `mynes.ini`, to `build/output/`. This changes
only product build delivery; App/Core behavior and the adopted four roots stay
unchanged. README and UX status now describe the verified routes rather than
the obsolete pre-qualification state.

Fresh x64 and x86 CTest JUnit suites each contain 103 tests, zero failures and
zero disabled tests. Native Window and Console receivers remain part of those
suites. The generated artifacts are PE x86-64 and PE i386, respectively. Their
SHA-256 identities and every remediation mapping are recorded in the
[T19 final audit](../etc/evidence/m3-t19-final-audit.md).

## Acceptance

M3 accepts the basic-use MyNes profile: mapper-0 ROM load, production
6502/PPU/controller execution, configured Window or text-only Console gameplay,
cooked monitor return and clean shutdown on x64 and x86. It does not claim
audio, PAL, mappers beyond NROM, save states, expansion devices or commercial
ROM compatibility. Those remain future milestone scope.
