# M5 T11 S10 HDC Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vhdc.c` `782c337a65d2e1453dbeee8e6c42170b4c66036fed3689c24b1243e9ed77d4c9`;
  `src/device/vhdc.h` `3831df0d8d5eb1faebc06825df88e24ab950c73c3881378e213b02fcc2425328`.
- Current M5 destination: `src/vm/machine/vhdc.c`
  `e953589bd7e0e8128cafca0d3f1ccf1534e06c0b1f5273bf3c78336292a2af18`;
  `src/vm/machine/vhdc.h`
  `7afa8c4f2f17c3c1ec82b993e895a9a50b7e9300759e7e036cd127524c4c2bc3`.
- Changes: preserved HDC logic and copyright notices; adjusted direct include
  paths and CMake ownership.
- Verification: `docs/verification/m5-t11-s10-hdc-move.md`.
