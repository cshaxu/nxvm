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

The supported compiler contract is GCC with C11 support.

The one supported current build route from the repository root is:

```powershell
cmake --preset mingw-gcc-x64-release
cmake --build --preset current-gcc
```

The configure preset creates `build/.ninja/gcc-x64-release`; `current-gcc`
builds the current developer artifact with Ninja parallelism8. `build/` itself
is not a build tree. Run `verify-current-specialized-gates` in that same tree.
The x86 equivalent is `build/.ninja/gcc-x86-release`.

ccache is optional: plain presets disable it and clear any previous launcher;
the corresponding `*-ccache` preset enables it in the same directory. Real
CMake configuration fixes its cache/temp/capacity in each compile command,
including direct builds. Directory retention, cleanup, task budgets and safety
have one authority:
[Build Tree Hygiene](../../../../rules/EXECUTION.md#build-tree-hygiene).
Query only this repository's cache, for example:

```powershell
cmake -E env "CCACHE_DIR=$PWD/build/.ccache" ccache --show-stats
```

Every implementation-task closure also builds the same product target for
32-bit Windows. Set `NXVM_I686_GCC` to an i686 MinGW-w64 `gcc.exe` and place
that compiler's `bin` directory first on `PATH`, then use
`mingw-gcc-x86-release` and `current-gcc-x86`. This is a second compiler
configuration, not a second source or product route.

## Core compiler feedback

Windows GCC Ninja Release can use profile-guided optimization for the CPU
instruction and Machine memory compilation units. Configure both normal Release
trees first. Train each architecture with its own compiler and the synthetic
repository workload. This requires Windows Python 3.9 or later.

```cmd
python tools/shared/train_core_pgo.py build/.ninja/gcc-x64-release build/.ninja/gcc-x64-release/core-pgo
python tools/shared/train_core_pgo.py build/.ninja/gcc-x86-release build/.ninja/gcc-x86-release/core-pgo
cmake -S . -B build/.ninja/gcc-x64-release "-DREPOSITORY_CORE_PGO_DIRECTORY:PATH=%CD%/build/.ninja/gcc-x64-release/core-pgo"
cmake -S . -B build/.ninja/gcc-x86-release "-DREPOSITORY_CORE_PGO_DIRECTORY:PATH=%CD%/build/.ninja/gcc-x86-release/core-pgo"
tools\build.cmd all --no-pause
```

The build menu reuses the configured trees and their selected profile directories.
Set `REPOSITORY_CORE_PGO_DIRECTORY` to an empty value for ordinary Release.
Refresh counters after a compiler or Core input change by configuring ordinary
Release and training into a fresh directory in the same tree. The training tool
accepts a new destination only. The usual build-tree cleanup procedure owns
obsolete counters. Keep generated counters local to their build tree.

The compiler rejects missing or unusable profiles. Ninja tracks each counter as
an input and reuses the compiled object when the inputs remain unchanged. The
current ccache bypasses the two profile-guided commands. The observable Machine
test target keeps its ordinary compiler flags. Continue to qualify both widths
and all receivers before publication.

Each fixed App's owner INI sits beside its EXE under `assets/<app>/`, not a
profile subdirectory. Product builds never rewrite it; media paths resolve
from that INI. No YAML compatibility route or generated replacement INI exists.
