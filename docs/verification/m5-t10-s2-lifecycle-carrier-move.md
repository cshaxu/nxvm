# M5 T10 S2 Lifecycle Carrier Move Verification

The original NXVM execution-carrier sources moved by `git mv`:

- `machine.c/.h` to `src/machine/vm/machine.c/.h`;
- `device/device.c/.h` to `src/machine/vm/device.c/.h`;
- `device/vmachine.c/.h` to `src/machine/vm/vmachine.c/.h`.

The only retained old-path headers are minimal forwarding includes for unmoved
callers. No old-path implementation remains in the CMake source set. The moved
implementation preserves the original `deviceStart` loop and `vmachineRefresh`
order; its direct controller dependencies remain explicit baseline dependencies
until T11.

`cmake --build build/mingw-gcc-x64 --target nxvm-m5-t10` passed with the
project MinGW-w64 GCC toolchain. The no-media `help`, `info`, `exit` regression
and delayed `debug`, `q`, `exit` regression passed with the original Console
markers. `nxvm-full-pc-profile-smoke` also built and linked successfully; its
runtime fixture arguments were not supplied, so no fixture behavior claim is
made.

No Console, debugger, display, input, media, boot, or thread behavior was
intentionally changed. T10 is still active, so no task-level artifact was
copied to `build/output`.

The M5 T12 S9 canonical-root follow-up moved the unchanged lifecycle pair to
`src/vm/machine/machine.c` and `.h`, repaired its direct callers, and reran the
GCC target plus finite CPU probe. The probe retained
`M5:T1:S1:CPU-PROBES:OK`.
