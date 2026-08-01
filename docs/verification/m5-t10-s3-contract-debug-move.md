# M5 T10 S3 P16 Contract Debug Move Verification

The M3 paused-state contract `src/core/debug.c/.h` moved by `git mv` to
`src/machine/core/contract/debug.c/.h`. The old header is a minimal forwarding
include. This is not the retained NXVM Console debugger.

The project MinGW-w64 GCC build passed for `nxvm-core-machine-debug-smoke` and
`nxvm-baseline-cpu-probe-smoke`. Their established debug and CPU-probe markers
passed. No guest execution or user-facing behavior changed.
