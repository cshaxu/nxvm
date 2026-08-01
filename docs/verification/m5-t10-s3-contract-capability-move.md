# M5 T10 S3 P8 Contract Capability Move Verification

The M3 CPU-capability contract `src/core/cpu_capability.c/.h` moved by
`git mv` to `src/machine/core/contract/cpu_capability.c/.h`. The old header is
a minimal forwarding include for existing consumers. The `nxvm-core` target now
compiles the moved implementation; this contract remains non-executing and is
not a second guest CPU implementation.

The project MinGW-w64 GCC build passed for `nxvm-core-cpu-capability-smoke`,
`nxvm-runtime-registry-smoke`, `nxvm-m5-t10`, and
`nxvm-baseline-cpu-probe-smoke`. The contract capability and registry smokes
passed their established markers. The real CPU probe also passed with its
established illegal-instruction diagnostic and `M5:T1:S1:CPU-PROBES:OK` marker.

No user-facing or guest execution behavior changed. T10 remains active, so no
task-level artifact was copied to `build/output`.
