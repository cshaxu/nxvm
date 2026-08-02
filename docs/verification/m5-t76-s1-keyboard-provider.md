# M5 T76 S1 Verification

Date: 2026-08-02

The full-PC request transport now binds its owning `nxvm_full_pc` as consumer
context and applies host keyboard state through that instance's frozen provider
slot. The legacy global keyboard facade has no production caller.

GCC 16.1.0 built `nxvm-0-5-0076` successfully. The following gates passed:

- `nxvm-vm-keyboard-firmware-smoke`: `M5:T41:S1:KEYBOARD-FIRMWARE:OK`.
- `nxvm-vm-keyboard-provider-isolation-smoke`:
  `M5:T76:S1:KEYBOARD-PROVIDER-ISOLATION:OK`.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: `M5:T70:S2:DOS-PROMPT:OK`.
- `verify-facade-ownership`: `M5:T75:FACADE-OWNERSHIP:OK` with the keyboard
  legacy facade limited to its implementation file.

Artifact: `build/output/nxvm_0_5_0076.exe`

SHA-256: `3EAD5324C1FC3151F65069A140370663B86C88E80D67C7B5000162EA921F9EFB`
