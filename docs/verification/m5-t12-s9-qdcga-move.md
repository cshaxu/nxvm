# M5 T12 S9 QDCGA Move Verification

`cmake --build build/mingw-gcc-x64 --target nxvm-m5-t10
nxvm-baseline-cpu-probe-smoke` completed with MinGW-w64 GCC, retaining existing
warnings. `nxvm-baseline-cpu-probe-smoke.exe` reported `#UD(0)` and
`M5:T1:S1:CPU-PROBES:OK`; `git diff --check` passed.

No QDX, display, input, BIOS, startup, Console, or debugger behavior changed.
