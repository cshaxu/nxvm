# M5 T12 S1 Platform Hub Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/platform/platform.c` `407c722066bfd6e3a5e80f35105f41063cf56603920e069e63c3f54bb9ebd46e`;
  `src/platform/platform.h` `d0b80296e610ff7688cdfb37d70add01a8fb9089b37e70b8871941b13b0d579f`.
- M5 destinations: `src/platform/vm/platform.c`
  `2d1af2b618a9669e9fbae087626eda0fe139f74bc95093304e8dcbebf24fe220`;
  `src/platform/vm/platform.h`
  `2bf4a5076911a31da57bf480021c5f599163c1079d344e94eb5dda274bd1ecd5`.
- Changes: preserved platform-hub logic and copyright notices; adjusted include
  paths, added a forwarding header, and added the source to the temporary
  runtime archive plus user-facing artifact source set.
- Verification: `docs/verification/m5-t12-s1-platform-hub-move.md`.
