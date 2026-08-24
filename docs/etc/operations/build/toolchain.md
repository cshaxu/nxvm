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

The one supported current build route from the repository root is:

```powershell
cmake --preset mingw-gcc-x64
cmake --build --preset current-gcc
```

The configure preset creates `build/mingw-gcc-x64`; the `current-gcc` build
preset builds the current developer artifact. The top-level `build/` directory
is a container, not a build directory. Run current gates through the separate
`current-gates-gcc` build preset. Other compilers or temporary build trees are
task-specific verification inputs, not supported development commands.

When `ccache` is installed, `mingw-gcc-x64-ccache` and `current-gcc-ccache`
provide an opt-in isolated build tree. They are an acceleration aid only; the
default route remains the supported route and never requires ccache.

## Binary Size Policy

GCC selection makes the development toolchain lighter; it does not by itself
guarantee a smaller executable. After M1 has a runtime target, release builds
will measure size and startup behavior. Candidate release flags are `-O2`,
`-ffunction-sections`, `-fdata-sections`, and linker garbage collection. Stripping
and static runtime linking require separate reproducibility and diagnostics
review; they are not enabled by default.
