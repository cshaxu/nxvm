# M5 T12 S9 VM Win32 Window Display Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/platform/win32/w32adisp.c` and `w32adisp.h`.
- Current destination: `src/vm/platform/win32/w32adisp.c`
  `D8C2391C1F9E01BAC614E56103B7521FAD03E604F0389918A237378D7779C045` and
  `src/vm/platform/win32/w32adisp.h`
  `48377D86065091F70189DEF0B7D65E09ABAE575F055D432E479DDAD89E14362F`.
- Ownership: direct VM display-device access makes this a VM platform backend.
- Changes: direct include paths and CMake source ownership only.
- Verification: `docs/verification/m5-t12-s9-win32-window-display-move.md`.
