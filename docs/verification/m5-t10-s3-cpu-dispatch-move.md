# M5 T10 S3 P7 CPU Dispatch Move Verification

The original NXVM 8086+ instruction dispatcher `device/vcpuins.c` moved by
`git mv` to `src/machine/core/vcpuins.c`. It is the only dispatcher
implementation in the user-facing target source set. Its only adaptations are
explicit include paths to the retained utility, lifecycle, and PIC headers and
to the moved port, RAM, and instruction-state headers.

No opcode, exception, trace, debugger, or execution-loop logic changed. The
existing single-session global binding remains temporary by the T10 plan;
unmoved controllers use explicit compatibility headers until their later moves.

The project MinGW-w64 GCC build passed for `nxvm-m5-t10`,
`nxvm-full-pc-profile-smoke`, and `nxvm-baseline-cpu-probe-smoke`. The finite
CPU probe passed with the established illegal-instruction diagnostic and
`M5:T1:S1:CPU-PROBES:OK` marker. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with their original Console
markers. The profile smoke executable was linked only; fixture arguments were
not supplied, so this step makes no guest-media behavior claim.

T10 remains active, so no task-level artifact was copied to `build/output`.
