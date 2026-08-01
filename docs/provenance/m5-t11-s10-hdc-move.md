# M5 T11 S10 HDC Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vhdc.c` `782c337a65d2e1453dbeee8e6c42170b4c66036fed3689c24b1243e9ed77d4c9`;
  `src/device/vhdc.h` `3831df0d8d5eb1faebc06825df88e24ab950c73c3881378e213b02fcc2425328`.
- M5 destinations: `src/machine/vm/vhdc.c`
  `c091e33e101da37faa086d75f448ea1c40e4b1d3852df80ec7db213c9e2936bd`;
  `src/machine/vm/vhdc.h`
  `91880a44360e97885c853c6458d6f8d0cf08243fd3378ca61c4adf6aa0e2c48b`.
- Changes: preserved HDC logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the VM CMake
  source set.
- Verification: `docs/verification/m5-t11-s10-hdc-move.md`.
