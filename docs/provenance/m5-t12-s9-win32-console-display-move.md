# M5 T12 S9 VM Win32 Console Display Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/platform/win32/w32cdisp.c` and `w32cdisp.h`.
- Current destination: `src/vm/platform/win32/w32cdisp.c`
  `EDC348515F5F33F716B15288BBA625AAC1DF5AF782BC3AD19EF96BEDC6EC66F4` and
  `src/vm/platform/win32/w32cdisp.h`
  `D17E696B715D5F2D83EB75F07D57E9221E7D6EC98ED0F9687B996BFB26AABAE0`.
- Ownership: direct VM display-device access makes this a VM platform backend.
- Changes: direct include paths and CMake source ownership only.
- Verification: `docs/verification/m5-t12-s9-win32-console-display-move.md`.
