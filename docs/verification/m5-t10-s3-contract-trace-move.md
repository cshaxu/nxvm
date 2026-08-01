# M5 T10 S3 P15 Contract Trace Move Verification

The bounded M3 contract `src/core/trace.c/.h` moved by `git mv` to
`src/machine/core/contract/trace.c/.h`. The old header is a minimal forwarding
include. The contract machine headers include the relocated trace contract
directly.

This finite event-ring contract remains distinct from prohibited raw instruction
recording and does not change NXVM guest execution or user-facing behavior.

The project MinGW-w64 GCC build passed for
`nxvm-core-machine-trace-smoke`, `nxvm-core-machine-debug-smoke`,
`nxvm-m5-t10`, and `nxvm-baseline-cpu-probe-smoke`. Both contract smokes
retained their established markers. The real CPU probe retained its established
illegal-instruction diagnostic and `M5:T1:S1:CPU-PROBES:OK` marker.

T10 remains active, so no task-level artifact was copied to `build/output`.
