# M5 T12 S9 VM Win32 Host Hub Move Verification

MinGW-w64 GCC built `nxvm-m5-t10` and `nxvm-baseline-cpu-probe-smoke`.
The probe reported the expected `#UD(0)` and `M5:T1:S1:CPU-PROBES:OK`; `git
diff --check` passed. No event-loop, keyboard, display, startup, Console, or
debugger behavior changed.
