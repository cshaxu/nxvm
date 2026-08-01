# M5 T12 S9 QDKEYB Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/device/qdx/qdkeyb.c` and `src/device/qdx/qdkeyb.h`.
- Current destinations: `src/vm/profile/default_profile/firmware/qdkeyb.c`
  `5B1DCD9D1A0E8AA238104CF3D588E2E535431FCCE7F49321DE2D981CD0BC10F0` and
  `src/vm/profile/default_profile/firmware/qdkeyb.h`
  `617C41089DFAE984C684B088B4CCB827D61B780F606355E8A94EF5BD5289B6EC`.
- Changes: preserved implementation; repaired direct includes and CMake source
  ownership only. The retained dispatcher includes the moved header by its
  canonical path.
- Verification: `docs/verification/m5-t12-s9-qdkeyb-move.md`.
