# M1 T1 S1 GCC Migration Design Verification

## Evidence

- NXVM commit: `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`.
- Toolchain: WinLibs x86_64 MSVCRT POSIX/SEH GCC 16.1.0; CMake 4.4.0; Ninja
  package located and reserved for T2.
- Fixture size and SHA-256 values match `docs/fixtures/m1-local-images.md`.
- A read-only, zero-patch compile of 37 Windows-selected NXVM C files succeeded
  with `-std=c11 -Wall -Wextra -Wpedantic`, linked against `user32` and `gdi32`.
  It produced 529 warnings and no errors.

## Result

The bounded migration design in
`docs/planning/m1-gcc-migration-design.md` is approved: **Go for M1 T2 S1**.
The warning inventory is recorded baseline debt and is not authorization for
cleanup during import.
