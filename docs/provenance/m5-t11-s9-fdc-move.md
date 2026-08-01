# M5 T11 S9 FDC Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vfdc.c` `99d1efe23d06cdcb1940dea632bbf8239d644dea92ffa0bbb3ba3737c3872534`;
  `src/device/vfdc.h` `e709014475b0b58f9b35973d52745eb37ec18d21045ac9c8227ca251bb637e94`.
- M5 destinations: `src/machine/vm/vfdc.c`
  `54ef118d0f00d2936a0f4c68380c2b6d51c1ff5f6961c17806c039edd65db36e`;
  `src/machine/vm/vfdc.h`
  `ffec267e8f076e1ff97fcb501669f58ecef9f3d8733fa4249f4b5fdb2c6bb15d`.
- Changes: preserved FDC logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the VM CMake
  source set.
- Verification: `docs/verification/m5-t11-s9-fdc-move.md`.
