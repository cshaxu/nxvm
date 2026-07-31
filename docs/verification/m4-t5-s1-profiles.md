# M4 T5 S1 Profile And Firmware Provider Verification

- The profile contract separates `nxvm.machine.*` full-machine topology from
  `ntvdm64.execution.*` non-booting DOS capability sets.
- `nxvm.full_pc` is recorded as the M3 adapter alias; M5's only new canonical
  profile is `nxvm.machine.pc_at_builtin`.
- The external-ROM manifest specifies local BYOR identity, hashes, mappings,
  reset vector, device requirements, and rejection conditions while prohibiting
  ROM/release inclusion. Award, Phoenix, IBM, Compaq, PCjs, and PC110-emu
  remain non-imported reference or user-supplied material as applicable.
- M5 breakdown contains a registry task and explicitly excludes external-ROM
  loading and additional machines. No implementation source changed.
