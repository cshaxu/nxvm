# M5 T10 S3 P5 CPU Control Move Verification

The original NXVM `device/vcpu.c` implementation moved by `git mv` to
`src/machine/core/vcpu.c`. It is the only `vcpu.c` implementation in the
user-facing target source set. Its only adaptations are explicit include paths
to the retained utility and instruction-dispatcher headers and to the already-
moved CPU state header.

The dispatcher remains at its original path for the next small source move, so
CPU reset, refresh, debugger accessors, and the single-session global binding
retain their original logic. The CPU probe adapter now explicitly links the
already-moved lifecycle library; this resolves the static-library dependency
introduced by the earlier lifecycle source move without changing runtime code.

The project MinGW-w64 GCC build passed for `nxvm-m5-t10`,
`nxvm-full-pc-profile-smoke`, and `nxvm-baseline-cpu-probe-smoke`. The finite
CPU probe passed with the established illegal-instruction diagnostic and
`M5:T1:S1:CPU-PROBES:OK` marker. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with their original Console
markers. The profile smoke executable was linked only; fixture arguments were
not supplied, so this step makes no guest-media behavior claim.

No Console, debugger, display, input, media, boot, or instruction behavior was
intentionally changed. T10 remains active, so no task-level artifact was copied
to `build/output`.
