# M5 T12 S9 VM Win32 Window Host Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/platform/win32/win32app.c` and `win32app.h`.
- Current destination: `src/vm/platform/win32/win32app.c`
  `33D5A0649287007A348A0415D866E125190D2446C6D6D3AC2E0D69FDC16F54C2` and
  `src/vm/platform/win32/win32app.h`
  `45328FD5A0F2787619E4C692EBA0C888059628E019AD99B6DEFD550443D8B0E5`.
- Ownership: its window, display, and machine-run threads are VM-specific.
- Changes: direct include paths and CMake source ownership only.
- Verification: `docs/verification/m5-t12-s9-win32-window-host-move.md`.
