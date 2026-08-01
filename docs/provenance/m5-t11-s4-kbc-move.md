# M5 T11 S4 KBC Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vkbc.c` `91ca494a6751b5822f3fb1f33ca59cba0dbae472c510e01cb4389fd1b2f83fa7`;
  `src/device/vkbc.h` `bc8db00cbebd910673b145bcd5dd3a0c21759dfdc43f29824be2e2b73d6539a5`.
- M5 destinations: `src/machine/core/vkbc.c`
  `61abb7577bba882d5ccdb07028ba52ba9954c504588d8113903cb996fcb4f963`;
  `src/machine/core/vkbc.h`
  `293d25c6e01a32f94f7b0b22004c67c65d44b49e5d1aa6fc1d0a1fe24d3be5b8`.
- Changes: preserved KBC logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the machine-core
  CMake source set.
- Verification: `docs/verification/m5-t11-s4-kbc-move.md`.
