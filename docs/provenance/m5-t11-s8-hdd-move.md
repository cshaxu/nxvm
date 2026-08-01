# M5 T11 S8 HDD Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vhdd.c` `e07ed4cb0f8346eb6a136947ba6a81ccd4609d4efb55523ccf5c8b0bc7c485f8`;
  `src/device/vhdd.h` `59651423f245f0b51c2292a1a67963ddafcdbddfafceebcf0ac15d5c7e68c9b1`.
- Current M5 destination: `src/vm/machine/vhdd.c`
  `669b3e08c9f0702517710e5b09476ce6d09e5a7a110834957e2d56ab382920d3`;
  `src/vm/machine/vhdd.h`
  `7f033737e98ef4cf462ab9981b3b22a1af49e782dfc08ddcb3e19c4944c67731`.
- Changes: preserved HDD logic and copyright notices; adjusted direct include
  paths and CMake ownership.
- Verification: `docs/verification/m5-t11-s8-hdd-move.md`.
