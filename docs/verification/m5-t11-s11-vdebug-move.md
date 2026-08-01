# M5 T11 S11 Hardware Debug-State Move Verification

The original NXVM hardware debug-state implementation now compiles from
`src/machine/core/vdebug.c`; its header is `src/machine/core/vdebug.h`. The old
header forwards retained callers to the new owner. VM lifecycle code includes
the state device directly, and CMake adds only the migrated implementation to
the core source set.

GCC built the user-facing target and focused smokes. The FDD-backed execution
lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke, no-media Console,
and debugger-prompt regressions passed. The raw recorder was not enabled. No
Console/debugger grammar, output, breakpoint, step, or trace behavior changed.
