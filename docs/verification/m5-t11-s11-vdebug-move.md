# M5 T11 S11 Hardware Debug-State Move Verification

The original NXVM hardware debug-state implementation now compiles from
`src/vm/machine/vdebug.c`; its header is `src/vm/machine/vdebug.h`. The retained
baseline header forwards callers to this owner. VM lifecycle code includes the
state device directly, and CMake adds only the migrated implementation to the
source set. Its direct `deviceStop()` control makes this VM-machine logic;
future shared debug services must use the separate Core contract.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. The canonical-root follow-up also
built the user-facing target and CPU probe, which reported
`M5:T1:S1:CPU-PROBES:OK`. The raw recorder was not enabled. No
Console/debugger grammar, output, breakpoint, step, or trace behavior changed.
