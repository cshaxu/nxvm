# M5 T10 S3 P13 Contract Memory Move Verification

The M3 contract `src/core/memory.c/.h` moved by `git mv` to
`src/machine/core/contract/memory.c/.h`. The old header is a minimal forwarding
include. The contract machine header now includes the relocated memory contract
directly.

This is the M3 range-checked memory contract, not the real NXVM `vram`
implementation already migrated to `src/machine/core/vram.c`. No guest
execution or user-facing behavior changed.

The project MinGW-w64 GCC build passed for
`nxvm-core-machine-instance-smoke`, `nxvm-core-machine-trace-smoke`,
`nxvm-m5-t10`, and `nxvm-baseline-cpu-probe-smoke`. Both contract smokes
retained their established markers. The real CPU probe retained its established
illegal-instruction diagnostic and `M5:T1:S1:CPU-PROBES:OK` marker.

T10 remains active, so no task-level artifact was copied to `build/output`.
