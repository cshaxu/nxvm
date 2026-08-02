# M5 T11 S9 FDC Move Verification

The original NXVM Intel 8272A FDC implementation now compiles from
`src/vm/machine/fdc.c`; its header is `src/vm/machine/fdc.h`. The retained
baseline header forwards BIOS callers to the new owner. `vmachine.c` includes
FDC directly, and CMake adds only the migrated implementation to the VM source
set. The existing Console media calls remain unchanged.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. The canonical-root follow-up also
built the user-facing target and finite CPU probe, retaining
`M5:T1:S1:CPU-PROBES:OK`. No FDC, DMA/IRQ, media, or user-visible NXVM behavior
changed.
