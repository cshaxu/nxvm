# Toolchain Contract

## Default Development Toolchain

Development and release targets are 64-bit Windows 7 through Windows 11. The
default x64 Windows development toolchain is MinGW-w64 GCC, CMake 3.23 or
later, and Ninja. This is a native Windows C toolchain and does not require
Visual Studio. Use a standalone MinGW-w64 distribution or a managed
installation that exposes `gcc.exe` and `ninja.exe` on `PATH` in an ordinary
Windows command environment. The default distribution uses MSVCRT to avoid
making the Universal CRT an implicit Windows 7 prerequisite.

If a future API, compiler runtime, or behavior difference makes all supported
versions impractical, Windows 11 usability takes priority. Record the Windows 7
gap as an evidence-backed `TODO(High)` or `TODO(Medium)` with its activation
condition; do not silently narrow the supported range.

The first supported compiler contract is GCC with C11 support. The M0 baseline
is WinLibs MinGW-w64 GCC 16.1.0, Ninja 1.13.2, and CMake 4.4.0. M1
project-owned compatibility and boundary code uses warnings equivalent to
`-Wall -Wextra -Wpedantic -Werror`; the imported whole-NXVM baseline records
its warning inventory before later units are migrated under that policy. Visual
Studio 2022/MSVC remains an optional compatibility build, not the primary
developer or release toolchain.

From the repository root:

```powershell
cmake --preset mingw-gcc-x64
cmake --build --preset m0-probe-gcc
Get-FileHash build/mingw-gcc-x64/probes/m1-text-exit.com -Algorithm SHA256
Get-Content build/mingw-gcc-x64/probes/m1-text-exit.json
```

The CMake configuration itself is the compiler smoke test: it must identify a
64-bit GCC C compiler and accept C11. The build generates the deterministic
M1 COM probe. The manifest must report marker `NTVDM64:M1:TEXT:OK`, exit code
`42`, the two DOS interrupt contracts, and the SHA-256 of the adjacent COM
file.

Every M1 baseline record also captures the exact `gcc`, `cmake`, and `ninja`
versions, executable SHA-256 values, host Windows edition/build, and target
architecture. A Windows 7 claim remains pending until the same record exists
from a Windows 7 host or an evidence-backed compatibility limitation is filed.

## Optional MSVC Verification

```powershell
cmake --preset vs2022-x64
cmake --build --preset m0-probe-msvc
```

MSVC may be used for periodic portability checks, but is not required for an
ordinary contributor or for the initial release path.

## Binary Size Policy

GCC selection makes the development toolchain lighter; it does not by itself
guarantee a smaller executable. After M1 has a runtime target, release builds
will measure size and startup behavior. Candidate release flags are `-O2`,
`-ffunction-sections`, `-fdata-sections`, and linker garbage collection. Stripping
and static runtime linking require separate reproducibility and diagnostics
review; they are not enabled by default.
