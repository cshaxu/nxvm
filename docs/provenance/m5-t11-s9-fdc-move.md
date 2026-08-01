# M5 T11 S9 FDC Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vfdc.c` `99d1efe23d06cdcb1940dea632bbf8239d644dea92ffa0bbb3ba3737c3872534`;
  `src/device/vfdc.h` `e709014475b0b58f9b35973d52745eb37ec18d21045ac9c8227ca251bb637e94`.
- Current M5 destination: `src/vm/machine/vfdc.c`
  `954d761b4a2b25df8eb27f926f99fb6d86fc618be55a76f720b1261b4fd3001e`;
  `src/vm/machine/vfdc.h`
  `8bbc56ebd71780ec560d92bd142d16a32775cb64ab5bfecdb7ece724f542b521`.
- Changes: preserved FDC logic and copyright notices; adjusted direct include
  paths and CMake ownership.
- Verification: `docs/verification/m5-t11-s9-fdc-move.md`.
