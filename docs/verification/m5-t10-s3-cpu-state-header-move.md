# M5 T10 S3 P4 CPU State Header Move Verification

The original NXVM `device/vcpu.h` moved by `git mv` to
`src/machine/core/vcpu.h`. The retained old-path header is a minimal forwarding
include for the unmoved instruction dispatcher, debugger, and controllers. The
moved header includes the already-moved shared machine-state header directly.

This step moves CPU data types and exported state declarations only. The CPU
implementation and instruction dispatcher remain at their old paths for the
following small execution-source moves; no CPU behavior was changed.

The project MinGW-w64 GCC build passed for `nxvm-m5-t10` and
`nxvm-full-pc-profile-smoke`. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with their original Console
markers. The profile smoke executable was linked only; fixture arguments were
not supplied, so this step makes no guest-media behavior claim.

T10 remains active, so no task-level artifact was copied to `build/output`.
