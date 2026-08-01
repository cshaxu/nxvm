# M5 T10 S3 P17 Contract PC Devices Move Verification

The M3 PC-device contract moved to `src/machine/core/contract/pc_devices.c/.h`.
The old header is a forwarding include. This is not the real NXVM controller
implementation planned for T11.

GCC build, `nxvm-core-pc-devices-smoke`, and the real CPU probe passed with
their established markers. No user-facing behavior changed.
