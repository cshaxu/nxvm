# M5 T11 S7 FDD Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vfdd.c` `83a27f0aac3fe546f3e2216d6f3380d360c595807f773d1599c95d55f11feced`;
  `src/device/vfdd.h` `69d4b3b27f22e1be36f7bad94c2895171cbc9d5d3f8ec9f90e30329fc99c397d`.
- M5 destinations: `src/machine/vm/vfdd.c`
  `c0ef3b53e3f0cf760560af19118cb8ab5616646907ff455d9ad1b00942369d72`;
  `src/machine/vm/vfdd.h`
  `c2d37b08af296dc0466b2d7e62ee406cfa27e4435ba35cb14c02dba840e8dfa5`.
- Changes: preserved FDD logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the VM CMake
  source set.
- Verification: `docs/verification/m5-t11-s7-fdd-move.md`.
