# M5 T12 S9 VM Win32 Host Hub Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/platform/win32/win32.c` and `win32.h`.
- Current destination: `src/vm/platform/win32/win32.c`
  `FCC7CDE6BD89EB370C2CC96EA15ADC54E873F1FE0E73E9BE71F81D9F605461E0` and
  `src/vm/platform/win32/win32.h`
  `B6AB8D4D26DD7CA48A122E03B7E0A87B296D62B94F95A7070210A313F9092B69`.
- Ownership: this host hub directly controls VM device state and is not an
  existing shared Platform implementation. It therefore belongs in `vm`.
- Changes: direct include paths and CMake ownership only; source encoding was
  retained while its line endings were normalized to repository LF.
- Verification: `docs/verification/m5-t12-s9-win32-hub-move.md`.
