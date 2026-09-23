# NXVM Product Binaries

Each NXVM profile deploys its versioned x64 and x86 EXE beside its `NXVM.ini`,
under `<profile>/`. The executable and INI are checked in together; CMake
updates only the selected profile's paired artifacts.

`build/` is not a product deployment location. Firmware, CMOS seeds, fonts,
guest media, and other protected inputs remain external BYOB assets. MyNES
uses its peer root `assets/binary-mynes/`.
