# M5 T11 S7 FDD Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vfdd.c` `83a27f0aac3fe546f3e2216d6f3380d360c595807f773d1599c95d55f11feced`;
  `src/device/vfdd.h` `69d4b3b27f22e1be36f7bad94c2895171cbc9d5d3f8ec9f90e30329fc99c397d`.
- Current M5 destination: `src/vm/machine/vfdd.c`
  `3648a46063b523f542ce4bea16f15f8f205e6dbe5f960790542c10dee0070a01`;
  `src/vm/machine/vfdd.h`
  `b8e3545319e7599d27ce1716fb240cc279b325f1f174b3873a7a9ef4488a9d66`.
- Changes: preserved FDD logic and copyright notices; adjusted direct include
  paths and CMake ownership.
- Verification: `docs/verification/m5-t11-s7-fdd-move.md`.
