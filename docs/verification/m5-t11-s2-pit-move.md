# M5 T11 S2 PIT Move Verification

The original NXVM 8254 PIT implementation now compiles from
`src/machine/core/vpit.c`; its header is `src/machine/core/vpit.h`. The old
header forwards retained BIOS callers to the new owner. `vmachine.c`, RAM, and
PIC include the new PIT header directly. CMake adds only the migrated PIT
implementation to the machine-core source set.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. No timer, IRQ, or user-visible NXVM
behavior changed.
