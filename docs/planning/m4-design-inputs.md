# M4 Design Inputs From M3

M3 leaves two explicit, testable profile paths for M4:

- `nxvm.full_pc`: retained single-session NXVM adapter with BIOS, CMOS, DMA,
  storage, VADP, and local fixture boot evidence. M4 assigns retained behavior
  to `firmware` versus `products/nxvm` and specifies the retained interactive
  NXVM Console surface. M4 does not add an nxvm process CLI.
- `ntvdm64.dos_minimal`: core-backed no-media profile with PIC/PIT, keyboard,
  text snapshots, presentation adapter, trace, and debug boundaries. M6 owns
  all DOS ABI decisions; M4 does not add DOS behavior.

M4 must preserve the legacy-adapter regression until M5 replaces it with an
explicit firmware/product composition. It must not treat the adapter as a
reusable core Machine or alter the M8 product-CLI boundary.

M4 also establishes that the retained descriptor is an adapter, not the final
machine model: M5 introduces only `nxvm.machine.pc_at_builtin`; future NXVM
machine profiles and ntvdm64 execution profiles use the separate profile
families defined in `docs/requirements/profiles.md`.

The retained CPU is partial i386 coverage, not a complete 80386 claim. M5 first
establishes capability probes and optional local Bochx/Bochs differential
verification under `docs/requirements/cpu-verification.md` before CPU-adjacent
migration.

The bounded M4 design tasks are in
`docs/planning/m4-firmware-nxvm-design-breakdown.md`.
