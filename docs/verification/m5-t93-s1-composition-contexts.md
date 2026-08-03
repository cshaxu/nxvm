# M5 T93 S1: VM Composition Contexts

## Result

VM composition owns debugger, Console, Console-target, and platform contexts.
The core debugger/utils static-library cycle is an explicit CMake rescan group.

## Verification

- `nxvm-current-gates-gcc`: passed.
- `nxvm-vm-two-session-isolation-smoke`:
  `M5:T73:S1:TWO-SESSION-ISOLATION:OK`.
- FDD and HDD retained gates passed.
- Artifact SHA-256: `88DAF41D138839C99C5189AD685CA9C7F0636DADED26EFF9848F2AD4946A0EDD`.
