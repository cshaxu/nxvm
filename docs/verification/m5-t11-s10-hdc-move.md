# M5 T11 S10 HDC Move Verification

The original NXVM HDD-controller implementation now compiles from
`src/vm/machine/vhdc.c`; its header is `src/vm/machine/vhdc.h`. The retained
baseline header forwards BIOS callers to the new owner. `vmachine.c` includes
HDC directly, and CMake adds only the migrated implementation to the VM source
set. The existing Console media calls remain unchanged.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. The canonical-root follow-up also
built the user-facing target and finite CPU probe, retaining
`M5:T1:S1:CPU-PROBES:OK`. No BIOS disk registration, HDD media, or
user-visible NXVM behavior changed.
