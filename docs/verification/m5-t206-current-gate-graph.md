# M5 T206: Current-Gate Graph De-Duplication

## Contract

`run-current-smokes` is the sole current-gate entry for registered smoke
execution. Smoke executables remain normal CMake targets and CTest cases; they
are not separately scheduled by the current-gates preset or legacy custom
wrappers.

## Evidence

- The obsolete Console lifecycle and core lifecycle custom targets, and the
  standalone preset reconfigure-smoke entry, are absent.
- `current.vm-session-reconfigure-smoke` remains registered; full
  `current-gates-gcc` passed 42/42 CTest smoke and 19/19 static gates.
- Artifact: `build/output/nxvm_0_5_0206.exe`; Console `EXIT` status 0;
  SHA-256 `60EEE0D2671073AB3B19567CDDD6266F70CAE74121B222FFC90C9F5D29139935`.
