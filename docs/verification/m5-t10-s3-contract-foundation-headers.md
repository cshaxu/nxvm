# M5 T10 S3 P9 Contract Foundation Headers Verification

The M3 contract `src/core/status.h` and `src/core/lifecycle.h` moved by
`git mv` to `src/machine/core/contract/`. Their old paths are minimal
forwarding headers, preserving existing consumers while the remaining contract
implementation is relocated in later small steps.

The project MinGW-w64 GCC build passed for
`nxvm-core-machine-lifecycle-smoke`, `nxvm-runtime-registry-smoke`,
`nxvm-m5-t10`, and `nxvm-baseline-cpu-probe-smoke`. The lifecycle and registry
smokes retained their established markers. The real CPU probe retained its
established illegal-instruction diagnostic and `M5:T1:S1:CPU-PROBES:OK` marker.

No user-facing or guest execution behavior changed. T10 remains active, so no
task-level artifact was copied to `build/output`.
