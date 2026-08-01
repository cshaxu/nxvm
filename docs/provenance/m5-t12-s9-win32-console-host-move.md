# M5 T12 S9 VM Win32 Console Host Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/platform/win32/win32con.c` and `win32con.h`.
- Current destination: `src/vm/platform/win32/win32con.c`
  `1951CC6DA8CCDB045FBE344E8D466701B0F40570C58087C1188F3E2416A6EE89` and
  `src/vm/platform/win32/win32con.h`
  `966D6A0AA31E5963C1FD51A35F9568C3A9A79CC64A8909ABB2BA49834233357F`.
- Ownership: its machine run and Console display threads are VM-specific.
- Changes: direct include paths and CMake source ownership only.
- Verification: `docs/verification/m5-t12-s9-win32-console-host-move.md`.
