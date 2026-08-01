# M5 T10 S3 P6 CPU Dispatch Header Move Verification

The original NXVM `device/vcpuins.h` moved by `git mv` to
`src/machine/core/vcpuins.h`. The retained old-path header is a minimal
forwarding include for the still-unmoved dispatcher implementation, debugger,
and controllers. The moved header includes the already-moved CPU state header
directly.

This step moves instruction-dispatch state and declarations only. The 8086+
dispatcher implementation remains at its original path for the following small
source move; no instruction semantics changed.

The project MinGW-w64 GCC build passed for `nxvm-m5-t10`,
`nxvm-full-pc-profile-smoke`, and `nxvm-baseline-cpu-probe-smoke`. The finite
CPU probe passed with the established illegal-instruction diagnostic and
`M5:T1:S1:CPU-PROBES:OK` marker. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with their original Console
markers. The profile smoke executable was linked only; fixture arguments were
not supplied, so this step makes no guest-media behavior claim.

T10 remains active, so no task-level artifact was copied to `build/output`.
