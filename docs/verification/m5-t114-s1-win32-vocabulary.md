# M5 T114 S1: Win32 Adapter Vocabulary

## Result

Win32 adapter public headers expose `C_VOID`, `WIN32_BOOL`, `WIN32_HANDLE`,
and `WIN32_HWND` through the private `win32_types.h` boundary. Retained
`win32app`, `win32con`, and `w32*` names are unchanged; raw SDK calls and
callback/message-loop types stay in adapter implementation files.

## Verification

- `cmake --preset mingw-gcc-x64` and
  `cmake --build --preset nxvm-current-gates-gcc`: passed.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0114.exe` accepted `help`, `info`, and `exit`; retained Console
  output and idle device state were observed.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0114]`.
- Developer artifact: `build/output/nxvm_0_5_0114.exe` (not a release;
  contains no guest media), SHA-256:
  `54E9AFE2D13B2B5E4A5B60F199DC7E8E0B560DA66701293F0E8E76ED88008245`.
