# M5 T11 S6 CMOS Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vcmos.c` `1c3c67767a6d2d2912ac41720c4032fb07e0f71d00814462c1b8f751785ad0b5`;
  `src/device/vcmos.h` `33552c1d4339b9471a89030f936f09fa5009a0a13e0e5dacaa1527ebcdf23f74`.
- M5 destinations: `src/machine/core/vcmos.c`
  `91ced4ce5c3f0270cd530d784c1959f97ea891baa1b344e589b7d02d6987019d`;
  `src/machine/core/vcmos.h`
  `f1346e036032c43a8802795493134afccd2539fd85e69a4ae151713a616159ed`.
- Changes: preserved CMOS logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the machine-core
  CMake source set.
- Verification: `docs/verification/m5-t11-s6-cmos-move.md`.
