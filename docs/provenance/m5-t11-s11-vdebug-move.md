# M5 T11 S11 Hardware Debug-State Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vdebug.c` `d8459e926b70de31b38351d71acffa01ec330fd290261960b4eaaf1af020ca8a`;
  `src/device/vdebug.h` `939705cb1827ea0959c41ed638e7995ffb15b74075d1cb8c9835c3770dc14b51`.
- M5 destinations: `src/machine/core/vdebug.c`
  `28a9c2d984c17475d201f89871f899e1122048ca6e13d8bde9cc0a169dbbf23d`;
  `src/machine/core/vdebug.h`
  `4b1f2a82a660464e39f00334fc8d4d6c38a8c91ad3e7aec7f977edb81ef97ba9`.
- Changes: preserved hardware debug-state logic and copyright notices; adjusted
  include paths, added a forwarding header, and added the implementation to
  the core CMake source set.
- Verification: `docs/verification/m5-t11-s11-vdebug-move.md`.
