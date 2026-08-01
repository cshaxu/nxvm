# M5 T12 S9 QDX Dispatcher Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/device/qdx/qdx.c` and `src/device/qdx/qdx.h`.
- Current destinations: `src/vm/profile/default_profile/firmware/qdx.c`
  `EE3DF6FD9C410CDE7C7900E2A651EC7401C8BC6CC931943E72CFE95129D5931A` and
  `src/vm/profile/default_profile/firmware/qdx.h`
  `E262D6BF581E5B059CA1CB1BBEED14C625834188737F18D107A60C232E7D28BD`.
- Changes: preserved implementation; repaired QDX subsystem direct header paths
  and CMake source ownership only.
- Verification: `docs/verification/m5-t12-s9-qdx-move.md`.
