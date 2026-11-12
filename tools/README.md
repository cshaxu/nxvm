# Tool Layout

- `shared/` contains repository-wide governance and neutral maintenance tools.
- `nxvm/` contains NXVM-only build, test, research, and release tooling.
- `mynes/` contains MyNES-only build, test, and release tooling.

Product tools may consume their own product documentation and assets, but may
not define a second implementation of shared Lib, Common, or x86 behavior.

## Build and publish on Windows

Double-click `build.cmd`, choose a product (or All), and wait for x86 and x64
publication to finish. The window stays open with the result. To automate:

```cmd
tools\build.cmd nxvm --no-pause
tools\build.cmd mynes --no-pause
tools\build.cmd all --no-pause
```

Products: `nxvm`, `my5160`, `my5170`, `mydeskpro386`, `mypc110`, `mynes`,
`myraidenii`. Executables go to their existing `assets/<product>/` directory;
versions come from the current product build declarations. INI and media are
never copied or overwritten. Publication also restores a missing EXE when the
compiler has no work to do. A failure returns nonzero and stops the sequence;
a preceding product/architecture may already have been published.

Required on PATH: CMake 3.23+, Ninja, x64 `gcc`, and
`i686-w64-mingw32-gcc`. `NXVM_I686_GCC` may specify the x86 compiler's full path.
ccache is optional. Existing PC BYOB inputs must be available in the sibling
`nxvm-assets/profiles-nxvm` archive or the configured `NXVM_PROFILE_ASSETS_ROOT`.
No tools or firmware are downloaded. MyNES/MyRaidenII selection does not require
PC firmware. Run only one build entry at a time: it reuses the same
`build/.ninja/gcc-x86-release` and `gcc-x64-release` trees across products.
The existing CMake cache policy fixes ccache under `build/.ccache`.

The build/publication chain uses CMD, CMake, Ninja and compiler tools only;
it does not invoke PowerShell. Independent test and governance commands are
not run by this entry. Successful compilation/publication is not a claim that
runtime or integration tests have run. All repository scripts remain LF.
