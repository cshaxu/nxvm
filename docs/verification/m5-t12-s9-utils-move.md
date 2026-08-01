# M5 T12 S9 Shared Utilities Move Verification

`cmake --build build/mingw-gcc-x64 --target nxvm-m5-t10
nxvm-baseline-cpu-probe-smoke` completed with MinGW-w64 GCC.

`nxvm-baseline-cpu-probe-smoke.exe` reported the expected `#UD(0)` marker and
`M5:T1:S1:CPU-PROBES:OK`. `git diff --check` passed. This move changes neither
the retained NXVM Console/debugger source nor its behavior.
