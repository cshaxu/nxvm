# M5 T197 Current Artifact Target Verification

## Closed Scope

The active CMake graph no longer registers historical task/version executable
targets. `add_current_vm_artifact()` creates only `vm-0-5-0197`; the current
GCC preset selects it. Existing files under `build/output/` remain historical
evidence and are not reproducible from current source under former names.

## Evidence

- `verify-current-artifact-target` emitted
  `M5:T197:S1:CURRENT-ARTIFACT-TARGET:OK`.
- `ninja -C build/mingw-gcc-x64 -t targets all` listed only
  `vm-0-5-0197` as a product artifact target.
- The full current GCC gate preset passed all 13 targets, including retained
  Console lifecycle, platform run-handle, lifecycle, dependency, ownership,
  Linux source-contract, and current-artifact-truthfulness gates.
- `nxvm_0_5_0197.exe` printed its `0.5.0197` identity and accepted piped
  `EXIT`. SHA-256:
  `C9FB6FC8E10B94203127BA225B0A35A5C439D2B66C140818426AE1B17E5EFAB1`.
