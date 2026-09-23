# T453 S1: Default Toolchain Baseline

The previous default CMake cache retained compiler generations from both
WinLibs and MSYS2 UCRT. A fresh default configuration records GCC 16.1.0,
`ar`, `gcc-ar`, `ranlib` and `gcc-ranlib` exclusively below the WinLibs
MSVCRT root. No machine-local path is committed; `CMakePresets.json` continues
to select `gcc` through the documented environment path.

The fresh complete build exposed one test-only missing declaration:
`vm_fdc_read_track_dos_smoke` uses the public FDC status constant but omitted
its direct owner header. Adding `core/machine/fdc.h` is the sole source change.
Every other current consumer already obtains the declaration through its owner
header.

The default artifact rebuilt as `nxvm_0_5_0448.exe` with SHA-256
`A294EA4AEBBB14C7D44B5F2141F6E457505C67808D25E985ECC37AE823BA7462`.
The clean-tree current-gate build and CTest replay completed; no failed or
notrun CTest record remains in the current build tree. Documentation governance
and diff hygiene pass.
