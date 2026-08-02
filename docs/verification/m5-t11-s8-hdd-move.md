# M5 T11 S8 HDD Move Verification

The original NXVM HDD implementation now compiles from
`src/vm/machine/hdd.c`; its header is `src/vm/machine/hdd.h`. The retained
baseline header forwards HDC callers to the new owner. `vmachine.c` includes
HDD directly, and CMake adds only the migrated implementation to the VM source
set. The existing Console media calls remain unchanged.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. The canonical-root follow-up also
built the user-facing target and finite CPU probe, retaining
`M5:T1:S1:CPU-PROBES:OK`. No media, HDC, or user-visible NXVM behavior changed.
