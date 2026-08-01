# M5 T11 S4 KBC Move Verification

The original NXVM 8042 KBC implementation now compiles from
`src/machine/core/vkbc.c`; its header is `src/machine/core/vkbc.h`. The old
header forwards retained callers to the new owner. `vmachine.c` includes KBC
directly, and CMake adds only the migrated implementation to the machine-core
source set. QDX keyboard queues and host input callers remain unchanged.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. No KBC or user-visible NXVM behavior
changed.
