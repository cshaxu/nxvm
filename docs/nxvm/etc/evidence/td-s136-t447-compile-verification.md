# M5 Td S136 T447 Compile Verification

## Verified Scope

This record completes the compile-verification requirement for Td S135 P1
`00ee2cde`. It verifies the entire current product target rather than only the
header's direct translation units.

## Result

- A fresh ignored `build/td135-verify-ucrt` Ninja configuration selected
  WinLibs UCRT GCC 16.1.0 as its C compiler.
- `ninja -C build/td135-verify-ucrt vm-0-5-0447` completed all 108 steps with
  exit status zero.
- The build compiled every `vm-composition` session source, including
  `session.c`, `machine_devices.c` and `model40_composition.c`, then linked
  `vm-0-5-0447.exe`.
- The temporary executable was 1,557,714 bytes and had SHA-256
  `F0555CE8CE32D7D4C7345D65E8C13298B03921818705247C8DB38D3E0B4660C9`.

The earlier MSYS2 GCC failure was isolated to its `cc1.exe` process returning
`0xC0000139`; it is not a Td S135 source failure. This record does not replace
the retained current-developer artifact or claim runtime test execution.
