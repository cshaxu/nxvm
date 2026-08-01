# M5 T11 S8 HDD Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vhdd.c` `e07ed4cb0f8346eb6a136947ba6a81ccd4609d4efb55523ccf5c8b0bc7c485f8`;
  `src/device/vhdd.h` `59651423f245f0b51c2292a1a67963ddafcdbddfafceebcf0ac15d5c7e68c9b1`.
- M5 destinations: `src/machine/vm/vhdd.c`
  `9fd81e3d627f27833fc3579ed014ddabd8743f239a042d2d05cb3c79e6ec85c4`;
  `src/machine/vm/vhdd.h`
  `98183a9825ab8d2ee77231b6947a8356cfd966218ce0aa723302557b148031a6`.
- Changes: preserved HDD logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the VM CMake
  source set.
- Verification: `docs/verification/m5-t11-s8-hdd-move.md`.
