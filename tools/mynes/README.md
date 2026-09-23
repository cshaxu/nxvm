# MyNES Tools

These tools are owned by the MyNES product. They may use the root shared
Lib/Common/x86 corpus, but must not introduce a second copy of its behavior.
Install CMake >= 3.23, Ninja and the appropriate MinGW compiler. No local
toolchain paths or game ROM bytes are stored in this repository.

Before running each architecture, prepend its compiler directory to the process
PATH so compiler helper executables resolve matching DLLs. For example, in a
fresh PowerShell for x86: $env:PATH = (Split-Path $env:MYNES_CC_X86) + ';' + $env:PATH.
Do not persist this into the system PATH. The presets verify pointer width.

Run from the repository root:

```powershell
cmake --preset mynes-gcc-x64-release
cmake --build --preset mynes-current-x64
cmake --build --preset mynes-tests-x64
```

For x86, the preset selects `i686-w64-mingw32-gcc`; set `MYNES_CC_X86` only to
override the compiler used by the standalone shared-component verifier, then
use the `mynes-gcc-x86-release` and `mynes-current-x86` presets. CTest is serial
because native Console probes must not compete. Assertions stay enabled in
Release; tests time out rather than hanging a run. Native GUI/Console tests
need an interactive Windows host. A headless failure is not a passing test.

Independent shared-component entries (substitute owned build directories and
compiler paths):

```powershell
cmake -S src/lib -B build/lib-only -G Ninja -DCMAKE_C_COMPILER="$env:MYNES_CC_X64"
cmake -S src/common -B build/common-only -G Ninja -DCMAKE_C_COMPILER="$env:MYNES_CC_X64" -DCOMMON_LIB_ROOT="$PWD/src/lib"
cmake -S test/lib -B build/lib-tests -G Ninja -DCMAKE_C_COMPILER="$env:MYNES_CC_X64" -DLIB_ROOT="$PWD/src/lib" -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc"
cmake -S test/common -B build/common-tests -G Ninja -DCMAKE_C_COMPILER="$env:MYNES_CC_X64" -DLIB_ROOT="$PWD/src/lib" -DCOMMON_ROOT="$PWD/src/common" -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc"
```

Build with cmake --build and run each test entry with ctest --test-dir and
--output-on-failure --timeout 180 --no-tests=error. Runtime cases retain their
explicit 30-second limits; the static layout self-test needs the longer limit.
Repeat with the x86 compiler.
Root paths are build inputs; changing consumer layout does not require source
edits. The four shared roots contain no x86 debugger or assembler tests.

For the full independent-entry proof on an architecture, run
`pwsh -File tools/mynes/Verify-SharedStandalone.ps1 -Architecture x64` (then x86).
It selects the matching environment compiler, builds all four entries in
separate ignored directories, verifies pointer width and C11 flags, and executes
both standalone suites serially with JUnit results. It restores its process PATH.
