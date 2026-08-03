# M5 T111 S1: C Scalar Vocabulary

## Result

Active project C declarations and definitions use the `C_*` scalar typedefs.
Headers that expose those types include `type.h` directly rather than relying
on include order. Fixed-width integer types remain unchanged, and string
literals, including firmware assembly text, retain their original vocabulary.

## Verification

- Source baseline: `1f59fda` (`M5 T110 S1`).
- Native-scalar scan outside `src/type.h` and `src/nxvm-baseline/`: clean for
  project C declarations and definitions.
- `cmake --preset mingw-gcc-x64` and
  `cmake --build --preset nxvm-current-gates-gcc`: passed.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0111.exe` accepted `help`, `info`, and `exit`; retained Console
  output and idle device state were observed.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0111]`.
- Developer artifact: `build/output/nxvm_0_5_0111.exe` (not a release;
  contains no guest media), SHA-256:
  `957914CE3581371AEB1692B8B59C18A6E93C2E28A78C232378DD24BD19755EC1`.
