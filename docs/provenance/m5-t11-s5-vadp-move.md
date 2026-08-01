# M5 T11 S5 VADP Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vvadp.c` `712e38e8051192e4b897ce60b101743e5709ecbdc821b78c3701e2a9be1edfc8`;
  `src/device/vvadp.h` `655c55701c6805919636dacd5f88e3025d03f45c3a7ccd160b11f31bf41871fb`.
- M5 destinations: `src/machine/core/vvadp.c`
  `83228e447b7a8bcaa4ee4af1d2d0176435dfc39e49fa9cecfc61ea268ebc16f8`;
  `src/machine/core/vvadp.h`
  `2d246438f43d4a305f1d5fb362c8d43874d8b540ec908d209424787eedd4bb19`.
- Changes: preserved VADP logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the machine-core
  CMake source set.
- Verification: `docs/verification/m5-t11-s5-vadp-move.md`.
