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

The bounded M4 design tasks are in
`docs/planning/m4-firmware-nxvm-design-breakdown.md`.
