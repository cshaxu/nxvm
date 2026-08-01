# M5 T12 S9 Source-Root Closure Verification

- `src` contains exactly `core`, `vm`, and `vdm`.
- CMake, product source, and test source contain no reference to an old source
  root. The obsolete `nxvm-baseline` artifact target is removed.
- Full MinGW-w64 GCC build completed for all configured targets. CTest has no
  registered tests in this configuration.
- The retained CPU probe reported `#UD(0)` and `M5:T1:S1:CPU-PROBES:OK`.
- No-media `help`, `info`, `exit`, and delayed `debug`, `q`, `exit` retained
  their expected original Console markers.
- `git diff --check` passed.
