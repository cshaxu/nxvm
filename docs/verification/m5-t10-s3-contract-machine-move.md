# M5 T10 S3 P11 Contract Machine Move Verification

The M3 contract machine `src/core/machine.c/.h` and private
`machine_impl.h` moved by `git mv` to `src/machine/core/contract/`. The two
old public/private header paths are minimal forwarding includes for still-
unmoved contract modules and existing API consumers. The `nxvm-core` target
now compiles the moved `machine.c` implementation.

This contract machine retains its documented finite lifecycle-model behavior;
it does not execute NXVM guest instructions and remains distinct from the real
moved NXVM executor. No public API or user-facing behavior changed.

The project MinGW-w64 GCC build passed for five contract-machine smokes,
`nxvm-m5-t10`, and `nxvm-baseline-cpu-probe-smoke`. The contract markers for
core contract, machine instance, lifecycle, trace, and debug all passed. The
real CPU probe retained its established illegal-instruction diagnostic and
`M5:T1:S1:CPU-PROBES:OK` marker.

T10 remains active, so no task-level artifact was copied to `build/output`.
