# M5 T11 S5 VADP Move Verification

The original NXVM VADP implementation now compiles from
`src/machine/core/vvadp.c`; its header is `src/machine/core/vvadp.h`. The old
header forwards retained QDX callers to the new owner. `vmachine.c` includes
VADP directly, and CMake adds only the migrated implementation to the
machine-core source set. QDX display state and Win32/Linux presentation remain
unchanged.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. No VADP or user-visible NXVM behavior
changed.
