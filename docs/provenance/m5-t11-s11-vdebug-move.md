# M5 T11 S11 Hardware Debug-State Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vdebug.c` `d8459e926b70de31b38351d71acffa01ec330fd290261960b4eaaf1af020ca8a`;
  `src/device/vdebug.h` `939705cb1827ea0959c41ed638e7995ffb15b74075d1cb8c9835c3770dc14b51`.
- Current M5 destination: `src/vm/machine/vdebug.c`
  `ca9f6dfdfa053a53cb30340adbf0331e380b12ed735e454616a043aba9c8205f`;
  `src/vm/machine/vdebug.h`
  `cbf6319ef73a32fa641617d0f3a8fc71ecbfd8b544669afe45723d4e2f5892d9`.
- Changes: preserved hardware debug-state logic and copyright notices; adjusted
  direct include paths and CMake ownership. The implementation remains a
  whole-VM execution device because it directly controls `deviceStop()`.
- Verification: `docs/verification/m5-t11-s11-vdebug-move.md`.
