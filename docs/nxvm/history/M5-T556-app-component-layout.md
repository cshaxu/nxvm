# M5 T556 — Five-App Component Layout

Owner admitted T556 on 2026-10-12. The
[proposal](../proposals/m5-app-component-layout.md) owns the requested target
layout and initial S1--S7 plan. S1 and MyNES S2 are accepted; My5160 S3 is
active.

Initial source inspection confirms two semantic boundaries require attention:
MyNES driver/media/debug access private hardware state, and DeskPro D4 files mix
register state with board registration and wiring. The three thin PC Apps can
be flattened through responsibility-based names and synchronized references.
S1's complete source/test/build map is retained in the
[inventory](../etc/evidence/t556-s1-app-layout-inventory.md). It fixes the
order as MyNES, My5160, My5170, NXVM and finally DeskPro386, and records the
one bounded MyNES Core-service boundary required before moving the Emulator
adapter. S1 was accepted in `c84ae39d3`; no production, test, build or
artifact changes were made by S1.

MyNES S2 is pushed as `693030c2e`. It establishes the requested
`{core,machine,product}` partition without changing Shared components or saved
data formats: the Machine adapter owns Emulator integration, media/battery I/O,
frame/audio adaptation and debug dispatch, while Core retains deterministic
hardware and exposes only bounded copied/direct operations required by that
adapter. Pure MMC3 behavior remains Core-owned. Both width-specific MyNES CTest
runs pass 57/57; the current 0044 x64/x86 artifacts were rebuilt. Manual use is
not implied by those automated routes.

My5160 S3 is complete. Its former `product/`, `profiles/` and `profiles/rom/`
source nesting and mirrored unit nesting are replaced by a flat App root with
role names (`main`, `product_binding`, `machine_plan`, `machine_config`,
`firmware_map`). The build target is `my5160-machine`. The root firmware
generator recognizes either an App-root entry or a retained `product/` entry;
this repaired the only migration regression, where the first flattened build
generated firmware under `src/product` instead of the My5160 App. The repaired
x64/x86 installer routes reach the installer, and the current artifact pair is
rebuilt. Shared-six and Core implementation remain unchanged.
