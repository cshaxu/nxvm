# NXVM Product Binaries

Each NXVM profile deploys its versioned x64 and x86 EXE beside its `NXVM.ini`,
under `<profile>/`. The executable and INI are checked in together; CMake
updates only the selected profile's paired artifacts. Keep only the latest
verified pair per runnable profile (currently four profiles, eight EXEs).
Delete superseded EXEs in the artifact delivery commit under
[Execution](../../docs/rules/EXECUTION.md); recover old versions from Git history.
Preserve each adjacent INI and all non-EXE assets.

`build/` is not a product deployment location. Firmware, CMOS seeds, fonts,
guest media, and other protected inputs remain external BYOB assets.
