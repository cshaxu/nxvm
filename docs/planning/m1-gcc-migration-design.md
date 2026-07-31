# M1 GCC Migration Design

## Decision

**Go:** import the full NXVM source at
`6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f` into the temporary M1 baseline and
build its existing Windows path with MinGW-w64 GCC 16.1.0. No NXVM source patch
is approved before raw-baseline observation.

## Assessment

NXVM contains 75 source files, including 37 C files. Its Autotools makefile
describes the Linux `pthread`/`ncurses` target; Windows source is present but
not represented by that build graph. A zero-patch Windows compile of all C files
except `platform/linux/`, using `-std=c11 -Wall -Wextra -Wpedantic` and linking
`user32` and `gdi32`, succeeded with 529 warnings. GCC accepts the existing
`unsigned __int64` spelling. The initial warning inventory is baseline evidence,
not a T2 cleanup backlog.

## T2 Build Contract

- Copy the complete `src/` tree to `src/nxvm-baseline/`; retain Linux sources
  but exclude them from the Windows executable target.
- Add a project-owned CMake Windows baseline target that selects all retained
  C sources other than `platform/linux/`, uses C11, and links `user32`/`gdi32`.
- Build the imported target without `-Werror`; record the warning count and
  compiler/tool executable hashes. New project-owned files retain the normal
  warning policy; imported warnings are not suppressed or fixed in M1.
- No compatibility header is needed for the observed GCC 16.1 baseline. If a
  later compiler defect requires one, it must be project-owned, documented,
  include no behavioral change, and stay within this narrow type/compiler
  compatibility class.

## Patch Budget

| Class | T2 allowance | Stop condition |
| --- | --- | --- |
| CMake/build composition | One baseline target and presets | New runtime behavior or Linux-source removal |
| Compiler compatibility | Zero expected; at most one project-owned shim | Source-wide edits or a semantic workaround |
| Warning handling | Inventory only | Warning cleanup mixed with baseline import |
| Runtime behavior | None | Any behavior-affecting change |

## Baseline Plan

T2 records the imported per-file manifest, notices, tool hashes, Windows build,
and FDD/HDD device commands. It runs each fixture with a bounded wall-clock and
no-progress budget, captures the selected display surface and terminal stop
reason, and never substitutes a missing fixture. `stop.com` and `reset.com`
remain unclassified unless a lawful guest-visible placement is observed.

Rollback is the verified commit before import; the sibling NXVM checkout remains
read-only throughout.
