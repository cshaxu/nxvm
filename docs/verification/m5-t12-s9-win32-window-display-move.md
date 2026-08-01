# M5 T12 S9 VM Win32 Window Display Move Verification

MinGW-w64 GCC built `nxvm-m5-t10` and `nxvm-baseline-cpu-probe-smoke`.
The probe reported `#UD(0)` and `M5:T1:S1:CPU-PROBES:OK`; `git diff --check`
passed. No display, input, startup, Console, or debugger behavior changed.
