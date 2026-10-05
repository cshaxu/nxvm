# NXVM Product Binaries

Each PC App deploys its versioned x64 and x86 EXE directly beside `NXVM.ini`
in `assets/my5160`, `assets/my5170`, `assets/mydeskpro386` or `assets/nxvm`.
Do not add a profile subdirectory. The executable and INI are checked in
together; CMake updates only the selected App's paired artifacts. Keep only
the latest verified pair per runnable App (four Apps, eight EXEs).
Delete superseded EXEs in the artifact delivery commit under
[Execution](../../docs/rules/EXECUTION.md); recover old versions from Git history.
Preserve each adjacent INI and all non-EXE assets.

`build/` is not a product deployment location. Firmware, CMOS seeds, fonts,
guest media, and other protected inputs remain external BYOB assets.
