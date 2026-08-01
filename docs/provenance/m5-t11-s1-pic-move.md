# M5 T11 S1 PIC Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, imported under the root MIT
  authorization recorded in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vpic.c` `89a1f10a0a4fcc8d1ae1aa634d515921c0e68b0349998c4c36d57005ff66fcc2`;
  `src/device/vpic.h` `aecaefc42df7fdb475e358089042a48f53763b9f366f4f5a604364071eda23de`.
- M5 destinations: `src/machine/core/vpic.c`
  `8e54adf45bea258dff14a27f8f67bde7c5b5cbe588cbb23b14a147dac1bed946`;
  `src/machine/core/vpic.h`
  `fa6d01070cf1c97f40f95d937e780ccf958b001a972fa0082aa66ea7caef9945`.
- Changes: preserved copyright notices and PIC logic; adjusted include paths,
  added an old-path forwarding header, and added the implementation to the
  machine-core CMake source set.
- Verification: `docs/verification/m5-t11-s1-pic-move.md`.
