# M5 T11 S7 FDD Move Verification

The original NXVM 1.44MB FDD implementation now compiles from
`src/machine/vm/vfdd.c`; its header is `src/machine/vm/vfdd.h`. The old header
forwards retained FDC callers to the new owner. `vmachine.c` includes FDD
directly, and CMake adds only the migrated implementation to the VM source set.
The existing Console media calls remain unchanged.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. No media, FDC, or user-visible NXVM
behavior changed.
