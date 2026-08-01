# M5 T11 S6 CMOS Move Verification

The original NXVM CMOS/RTC implementation now compiles from
`src/machine/core/vcmos.c`; its header is `src/machine/core/vcmos.h`. The old
header forwards retained callers to the new owner. `vmachine.c` includes CMOS
directly, and CMake adds only the migrated implementation to the machine-core
source set. Existing local clock and BIOS registration behavior is unchanged.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. No CMOS or user-visible NXVM behavior
changed.
