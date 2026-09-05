# M5 T519: 8042 Keyboard-Controller And Board-Integration Reclosure

The retained proposal defines the approved scope.  T519 closed the shared
8042/keyboard/IRQ1 repair required by the IBM 5170 Rev-3 ROM; no BIOS or
profile-specific keyboard workaround was added.

## S1 Acceptance

- KBC command-byte handling now distinguishes the board keyboard-inhibit
  signal from serial keyboard data.  Releasing the board inhibit begins BAT
  through the same delayed keyboard result, KBC output-buffer and PIC IRQ1
  path used by ordinary keyboard traffic.
- Console input uses the same native `INPUT_RECORD` conversion path that the
  focused input proof exercises; F1 and F3 reach the guest without a separate
  host-key FIFO.
- The 5170 external-ROM 360 KB and 1.2 MB sessions reached their declared
  installer terminals.  The owner manually confirmed the synchronized 0519
  artifact booted and that installer F3 operated correctly.
- Full repository-only unit passed 304/304.  The two focused external-ROM
  rows, KBC/input proofs, documentation governance, and actual-diff review
  passed.  Stripped developer artifacts are
  `nxvm_0_5_0519_x64.exe`
  (`4AB8A40AC8552DF01B5803BA256448FF51C717284EBD92D4E3528D6BED6EEA79`) and
  `nxvm_0_5_0519_x86.exe`
  (`B992D7E30F2E5A1F7F3FAED622934F12125086EB68F49AD7978881A5CD7A870B`).

## Accepted Commits

`092fefd4`, `6a07b9dc`, `46fb52b3`, `1e38d4d4`, `0921f07e`, `3bc79967`,
`f6ddef93`, `837d3592`, `268dcbc4`, `ce963797`, and `614f1811` record the
bounded repair, artifact synchronization and documentation normalization.
