# M5 T11 S2 PIT Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vpit.c` `0d076ea2740cf70ea699bd534294a1c295abe5b2bb09f1a384b35823654614e3`;
  `src/device/vpit.h` `a69679041d16a11eb9ac043227bd9a744c3ff4cb80cc22c5a078e4c605f439e4`.
- M5 destinations: `src/machine/core/vpit.c`
  `3d3687902fb0b9a62a59d4973f606028d2eb32b7c3eb59e00bb2278eb2ee5825`;
  `src/machine/core/vpit.h`
  `f2eb91658822a57cf2f8b84a1c8fc755380435b5a1ef358a111197721a34792e`.
- Changes: preserved PIT logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the machine-core
  CMake source set.
- Verification: `docs/verification/m5-t11-s2-pit-move.md`.
