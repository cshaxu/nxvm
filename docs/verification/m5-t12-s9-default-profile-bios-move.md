# M5 T12 S9 Default-Profile BIOS Move Verification

`cmake --build build/mingw-gcc-x64 --target nxvm-m5-t10
nxvm-baseline-cpu-probe-smoke` completed with MinGW-w64 GCC. The pre-existing
GCC warnings are retained.

`nxvm-baseline-cpu-probe-smoke.exe` reported the expected `#UD(0)` marker and
`M5:T1:S1:CPU-PROBES:OK`; `git diff --check` passed. No BIOS, POST, startup,
Console, debugger, input, or display behavior was changed.
