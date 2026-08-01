# M5 T11 S5 VADP Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vvadp.c` `712e38e8051192e4b897ce60b101743e5709ecbdc821b78c3701e2a9be1edfc8`;
  `src/device/vvadp.h` `655c55701c6805919636dacd5f88e3025d03f45c3a7ccd160b11f31bf41871fb`.
- Current M5 destination: `src/vm/machine/vvadp.c`
  `5514b54fadbaa93a2f4f890c8e775e0a7e0d43c9fa0b8443017428d72ef18a66d`;
  `src/vm/machine/vvadp.h`
  `159ebf55d358d38b7d34a98d402590e92b9c055c421509a877d7b417d3d9a496`.
- Changes: preserved VADP logic and copyright notices; adjusted direct include
  paths and CMake ownership. The VADP BIOS INT 10h registration makes this a
  VM-machine source, not a shared-core device.
- Verification: `docs/verification/m5-t11-s5-vadp-move.md`.
