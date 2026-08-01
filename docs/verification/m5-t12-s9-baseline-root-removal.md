# M5 T12 S9 Baseline Root Removal Verification

- `rg` found no formal source or CMake reference to `src/nxvm-baseline`.
- MinGW-w64 GCC built `nxvm-m5-t10` and `nxvm-baseline-cpu-probe-smoke` after
  the source glob and include-root dependency were removed.
- The CPU smoke reported the expected `#UD(0)` and `M5:T1:S1:CPU-PROBES:OK`.
- No-media `help`, `info`, `exit` retained the original banner, command text,
  and device-info markers. Delayed interactive `debug`, `q`, `exit` retained
  the `Console> -` debugger boundary and returned to `Console>`.
- `git diff --check` passed.
