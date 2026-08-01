# M5 T12 S9 VM Linux Host Hub Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/platform/linux/linux.c` and `linux.h`.
- Current destination: `src/vm/platform/linux/linux.c`
  `58E5DC27AD7D63E422F985A1F9B41554F5C7942D31AC85B40E8A3FDEE26A9E66` and
  `src/vm/platform/linux/linux.h`
  `26F20929E73F27987D79209A48F72723F111AD88FC083DC6D0E124015DFD7B0B`.
- Ownership: this host hub selects the VM device loop and is not currently a
  reusable VDM host-service implementation.
- Changes: direct include paths only; Linux remains excluded from Windows GCC.
- Verification: `docs/verification/m5-t12-s9-linux-hub-move.md`.
