# M5 T10 S3 P14 Contract Port Move Verification

The M3 contract `src/core/port.c/.h` moved by `git mv` to
`src/machine/core/contract/port.c/.h`. The old header is a minimal forwarding
include. The contract machine headers include the relocated port contract
directly.

This typed callback contract remains distinct from the real NXVM `vport`
implementation already migrated to `src/machine/core/vport.c`. No guest
execution or user-facing behavior changed.

The project MinGW-w64 GCC build passed for
`nxvm-core-machine-instance-smoke`, `nxvm-core-machine-trace-smoke`,
`nxvm-m5-t10`, and `nxvm-baseline-cpu-probe-smoke`. Both contract smokes
retained their established markers. The real CPU probe retained its established
illegal-instruction diagnostic and `M5:T1:S1:CPU-PROBES:OK` marker.

T10 remains active, so no task-level artifact was copied to `build/output`.
