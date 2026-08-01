# M5 T12 S9 QDCGA Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/device/qdx/qdcga.c` and `src/device/qdx/qdcga.h`.
- Current VM default-profile firmware destinations:
  `src/vm/profile/default_profile/firmware/qdcga.c`
  `72B3968364E8F2B300CEF7CC1C82BD4607390C833EC42B98E5E3B25BB0FB813D` and
  `src/vm/profile/default_profile/firmware/qdcga.h`
  `0AD43BC945B66158D7341BA55140E3716C2AA7BA4B158B04244B1177CF6D632F`.
- Changes: preserved QDCGA logic and copyright notices; repaired only direct
  include paths and CMake ownership. The retained QDX dispatcher now includes
  the moved public header by its canonical path.
- Verification: `docs/verification/m5-t12-s9-qdcga-move.md`.
