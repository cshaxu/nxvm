# M5 T115 S1: Debugger Input Flush Capability

## Result

The core debugger now calls a caller-provided input provider. VM composition
binds that provider to `vm_platform_input_flush_console_input`; Win32 flushes
only a handle accepted by `GetConsoleMode`, so redirected stdin is untouched.
No active `fflush(stdin)` or `fflush(STD_STDIN)` call remains.

## Verification

- Static scan for `fflush(stdin)` and `fflush(STD_STDIN)`: clean.
- `cmake --preset mingw-gcc-x64` and
  `cmake --build --preset nxvm-current-gates-gcc`: passed.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0115.exe` accepted `help`, `info`, and `exit`; retained Console
  output and idle device state were observed.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0115]`.
- Developer artifact: `build/output/nxvm_0_5_0115.exe` (not a release;
  contains no guest media), SHA-256:
  `71DEBC7B049223C96F4C0AF120062EB69217701E7E2F4B7FA0A8838832C1C345`.
