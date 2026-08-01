# M5 T10 S3 P1 Port Move Verification

The original NXVM `device/vport.c/.h` implementation moved by `git mv` to
`src/machine/core/vport.c/.h`. The retained old-path header is a minimal
forwarding include for unmoved callers. `vport.c` has no remaining old-path
implementation in the CMake source set.

The only implementation adaptation is its include path to the retained NXVM
utility header; the header's dependency on unmoved global machine state is
explicit. This preserves the original single-session port-dispatch binding.

The project MinGW-w64 GCC build passed for `nxvm-m5-t10` and
`nxvm-full-pc-profile-smoke`. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with their original Console
markers. The profile smoke executable was linked only; fixture arguments were
not supplied, so this step makes no guest-media behavior claim.

No Console, debugger, display, input, media, boot, or instruction behavior was
intentionally changed. T10 remains active, so no task-level artifact was copied
to `build/output`.
