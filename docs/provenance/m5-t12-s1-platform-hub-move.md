# M5 T12 S1 Platform Hub Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/platform/platform.c` `407c722066bfd6e3a5e80f35105f41063cf56603920e069e63c3f54bb9ebd46e`;
  `src/platform/platform.h` `d0b80296e610ff7688cdfb37d70add01a8fb9089b37e70b8871941b13b0d579f`.
- Current M5 destination: `src/vm/platform/platform.c`
  `e9b821f4ae89170e16b31e42977604968998f5f0251fd244d27cda1356b0746c`;
  `src/vm/platform/platform.h`
  `19c0d05c8ddda1c97c6c2e371502ef4315564ce1367b758f464ccbebcc9584e8`.
- Changes: preserved platform-hub logic and copyright notices; adjusted direct
  include paths and CMake ownership.
- Verification: `docs/verification/m5-t12-s1-platform-hub-move.md`.
