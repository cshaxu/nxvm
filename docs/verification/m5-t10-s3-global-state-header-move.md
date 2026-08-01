# M5 T10 S3 P3 Shared Machine-State Header Move Verification

The original NXVM `device/vglobal.h` moved by `git mv` to
`src/machine/core/vglobal.h`. The retained old-path header is a minimal
forwarding include for unmoved callers. The moved header now explicitly
includes the retained NXVM global header instead of relying on its former
relative path.

The already-moved RAM and port headers now include the moved machine-state
header directly. All remaining devices keep their existing include spelling
through the old-path forwarder, so this header-only step does not alter their
source behavior.

The project MinGW-w64 GCC build passed for `nxvm-m5-t10` and
`nxvm-full-pc-profile-smoke`. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with their original Console
markers. The profile smoke executable was linked only; fixture arguments were
not supplied, so this step makes no guest-media behavior claim.

No Console, debugger, display, input, media, boot, or instruction behavior was
intentionally changed. T10 remains active, so no task-level artifact was copied
to `build/output`.
