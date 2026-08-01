# M5 T11 S3 DMA Move Verification

The original NXVM 8237A DMA implementation now compiles from
`src/machine/core/vdma.c`; its header is `src/machine/core/vdma.h`. The old
header forwards retained FDC/FDD/HDD callers to the new owner. `vmachine.c`
includes DMA directly, and CMake adds only the migrated implementation to the
machine-core source set.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. No DMA callback or user-visible NXVM
behavior changed.
