# M5 T11 S1 PIC Move Verification

The original NXVM 8259A PIC implementation now compiles from
`src/machine/core/vpic.c`; its header is `src/machine/core/vpic.h`. The old
header is a forwarding compatibility layer for retained PIT, FDC, and keyboard
callers. `vmachine.c` and the CPU instruction dispatcher include the new owner
directly. CMake adds only the migrated PIC implementation to the machine-core
source set.

GCC built the user-facing target plus the focused lifecycle, CPU, and full-PC
smokes. The FDD-backed execution lifecycle, sequential CPU probe, FDD/HDD
reset-vector smoke, no-media Console, and debugger prompt regressions passed.
No controller behavior or user-visible NXVM behavior changed.
