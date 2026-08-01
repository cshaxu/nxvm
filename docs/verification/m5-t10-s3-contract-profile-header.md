# M5 T10 S3 P10 Contract Profile Header Verification

The M3 contract `src/core/profile.h` moved by `git mv` to
`src/machine/core/contract/profile.h`. The old path is a minimal forwarding
header for the still-unmoved contract machine implementation.

The project MinGW-w64 GCC build passed for
`nxvm-core-machine-lifecycle-smoke`, `nxvm-m5-t10`, and
`nxvm-baseline-cpu-probe-smoke`. The contract lifecycle smoke retained its
established marker. The real CPU probe retained its established
illegal-instruction diagnostic and `M5:T1:S1:CPU-PROBES:OK` marker.

No user-facing or guest execution behavior changed. T10 remains active, so no
task-level artifact was copied to `build/output`.
