# M5 T12 S9 QDDISK Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/device/qdx/qddisk.c` and `src/device/qdx/qddisk.h`.
- Current destinations: `src/vm/profile/default_profile/firmware/qddisk.c`
  `D1D8B8AA6A5EAD49565DE2A1FFAFF574310A51A7EEDA85070054755EAE93F570` and
  `src/vm/profile/default_profile/firmware/qddisk.h`
  `6548A5861CBBCD82B01068FDBBA93082E9E2F9F3D8890068D151EAE8D9DDC64C`.
- Changes: preserved implementation; repaired direct includes and CMake source
  ownership only. The retained dispatcher includes the moved header by its
  canonical path.
- Verification: `docs/verification/m5-t12-s9-qddisk-move.md`.
