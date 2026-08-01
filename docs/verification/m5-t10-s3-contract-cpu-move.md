# M5 T10 S3 P12 Contract CPU Move Verification

The M3 contract `src/core/cpu.c/.h` moved by `git mv` to
`src/machine/core/contract/cpu.c/.h`. The old header is a minimal forwarding
include. The contract machine headers now include the relocated CPU contract
directly.

This is the finite M3 state contract, not the real NXVM `vcpu` executor that
already lives in `src/machine/core/vcpu.c`. No guest instruction behavior or
user-facing behavior changed.

The project MinGW-w64 GCC build passed for
`nxvm-core-machine-instance-smoke`, `nxvm-core-machine-debug-smoke`,
`nxvm-m5-t10`, and `nxvm-baseline-cpu-probe-smoke`. Both contract smokes
retained their established markers. The real CPU probe retained its established
illegal-instruction diagnostic and `M5:T1:S1:CPU-PROBES:OK` marker.

T10 remains active, so no task-level artifact was copied to `build/output`.
