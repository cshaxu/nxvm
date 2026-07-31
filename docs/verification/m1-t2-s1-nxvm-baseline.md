# M1 T2 S1 NXVM Baseline Verification

## Import And Build

- Imported source: all 75 files in NXVM `src/` at commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`.
- Per-file provenance and SHA-256 manifest:
  `docs/provenance/m1-nxvm-baseline.md`.
- Hash comparison against the read-only sibling source found zero mismatches.
- Windows host: Windows 11 Pro 10.0.26200, 64-bit.
- GCC: WinLibs x86_64 MSVCRT POSIX/SEH 16.1.0,
  `7222d1a1d37b6bfb86b72c2645d14d855717761e7382c3bf6da8824f218dc1ad`.
- CMake 4.4.0:
  `4510f1883dfad3238602bae9a7a4b441fb4a931a643604aa44336710b6be5f6f`.
- Ninja 1.13.2:
  `e52a7ad9538d9618c67a0bd777964e2eec8a30f68b810a2f6adce1f2daf847b8`.
- `cmake --preset mingw-gcc-x64` and
  `cmake --build --preset m1-baseline-gcc` completed successfully.
- `nxvm-baseline.exe` SHA-256:
  `90ec2860a14a4e998cea6b85214959da07fcb28c5a2dfe9608f1618e57875363`.
- GCC recorded 529 imported-baseline warnings under
  `-Wall -Wextra -Wpedantic`; no warning cleanup entered M1.
- The copied source has pre-existing space/tab indentation. A path-scoped Git
  whitespace exemption preserves byte identity for this baseline only; all
  project-owned paths remain subject to `git diff --check`.
- Piped `exit` completed the Console lifecycle with exit status 0.

## Fixture Runs

The fixture identities matched `docs/fixtures/m1-local-images.md`. Each run
selected the Win32 Console display, configured the named local fixture, chose
the matching boot device, and called `start`. Standard-output redirection does
not capture the Win32 Console guest screen, so execution checkpoints use the
NXVM instruction recorder.

| Scenario | Device checkpoint | Execution checkpoint | Budget and stop reason |
| --- | --- | --- | --- |
| FDD | `Floppy disk inserted.` | 469,926 trace records; reset vector `F000:FFF0`; final recorded DOS `INT 21h` at line 469,925 | 10 seconds; CPU continued to make progress; watchdog terminated test process. |
| HDD | `Hard disk connected.` | 372,354 trace records; reset vector `F000:FFF0`; DOS `INT 21h` observed through line 224,602 | 10 seconds; CPU continued to make progress; watchdog terminated test process. |

The first and last instruction records are preserved only in ignored `build/`
output. No trace, image, or local absolute path is committed. `stop.com` and
`reset.com` were not placed in a guest-visible filesystem and remain
unclassified; they are not M1 exit conditions.

## Result

M1 is complete. The required immutable M1 snapshot branch is created from this
verified commit before M2 begins. M2 may use the recorded behavior and source
provenance, but must not treat the temporary baseline coupling as final
architecture.
