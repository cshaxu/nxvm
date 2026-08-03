# M5 T90 S1: VM Platform Surface Contexts

## Result

Each Win32 renderer now owns its buffer, GDI objects, and displayed generation
through a VM platform context. Windows Console and Linux curses terminal use
the shared atomic host-surface lease contract; immutable keyboard and character
maps remain safely shared. No VDM platform implementation exists to change.

## Verification

- `nxvm-vm-platform-surface-context-smoke`:
  `M5:T90:S1:SURFACE-CONTEXT:OK`.
- `ntvdm64-core-platform-host-surface-smoke`:
  `M5:T89:S1:HOST-SURFACE-LEASE:OK`.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`:
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm-vm-debug-pause-boundary-smoke D:\\fdd.img`:
  `M5:T45:S1:PAUSE-BOUNDARY:OK`.
- `nxvm-vm-hdd-authority-smoke D:\\hdd.img`:
  `M5:T36:S1:HDD-AUTHORITY:OK`.
- `nxvm-current-gates-gcc`: passed.

`build/output/nxvm_0_5_0090.exe` is the local GCC developer artifact:
SHA-256 `F68B11521089F90F662B0193D14680B84F9EC19AAD23644E8D106C04C09A86F3`.
Its banner is `Neko's x86 Virtual Machine [0.5.0090]`.
