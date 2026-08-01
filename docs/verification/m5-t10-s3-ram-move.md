# M5 T10 S3 P2 RAM Move Verification

The original NXVM `device/vram.c/.h` implementation moved by `git mv` to
`src/machine/core/vram.c/.h`. The retained old-path header is a minimal
forwarding include for unmoved callers. `vram.c` has no remaining old-path
implementation in the CMake source set.

The moved source uses explicit paths for the retained NXVM utility, PIT, and
global-state headers, plus the already-moved port header. This preserves the
original RAM allocation, A20, and port `0x92` behavior while the CPU and device
consumers remain unmoved.

The project MinGW-w64 GCC build passed for `nxvm-m5-t10` and
`nxvm-full-pc-profile-smoke`. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with their original Console
markers. The profile smoke executable was linked only; fixture arguments were
not supplied, so this step makes no guest-media behavior claim.

No Console, debugger, display, input, media, boot, or instruction behavior was
intentionally changed. T10 remains active, so no task-level artifact was copied
to `build/output`.
