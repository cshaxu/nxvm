# M5 T11 S5 VADP Move Verification

The original NXVM VADP implementation now compiles from
`src/vm/machine/vvadp.c`; its header is `src/vm/machine/vvadp.h`. The retained
baseline header forwards QDX callers to this owner. `vmachine.c` includes VADP
directly, and CMake adds only the migrated implementation to the source set.
VADP registers BIOS INT 10h, so its VM-machine ownership preserves the same
QDX display state and Win32/Linux presentation behavior.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. The canonical-root follow-up also
built the user-facing target and CPU probe, which reported
`M5:T1:S1:CPU-PROBES:OK`. No VADP or user-visible NXVM behavior changed.
