# M5 T556 — Five-App Component Layout

Owner admitted T556 on 2026-10-12. The
[proposal](../proposals/m5-app-component-layout.md) owns the requested target
layout and ordered S plan. S1--S7 are accepted. Owner admitted S8 comprehensive
quality audit on 2026-10-13, with confirmed repairs assigned to S9; T556 remains open.

S8 audit P1 `a28cac691` is accepted after coordinator inspection of its five
documentation files and finding-to-receiver mapping. Static evidence is not
presented as new runtime proof. S9 is admitted for A1--A5; A6 remains a
separate shared-rule Td receiver. No S9 implementation has been delivered yet.

S7 repair and delivery are pushed as `307cf1bd6` and `2df276f8c`.
The [S7 evidence](../etc/evidence/t556-s7-deskpro-boot-review.md) preserves
unattended dual-width installer proof and the distinction between complete-case
passing evidence and static tests retried after parallel-suite timeouts.
The [S8 audit](../etc/evidence/t556-s8-quality-audit.md) records confirmed
governance, documentation, registration and D4 boundary gaps without modifying
implementation; its S9 receiver preserves final whole-repository qualification.

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

My5170 S4 is complete. The fixed AT descriptor/plan, entry and binding now use
the flat App-root layout; its six matching unit sources and one integration
source no longer retain obsolete component directories. The library target is
`my5170-machine`. Current-at x64/x86 complete graphs build successfully; all
ten My5170 routes pass on both widths, including the external floppy installer
boot. The x64 artifact changed and was republished; the rebuilt x86 artifact
is deterministic and unchanged. The prior My5160 P2 commit accidentally
included S4's already-staged pure renames; this record preserves that factual
history while the following S4 completion commit owns the remaining updates.

NXVM S5 is complete. Its fixed default AT App is flat except for the retained
project-authored `firmware/` component: direct App-root roles are `main`,
`product_binding`, `machine_plan` and `machine_config`, with matching direct
unit receivers and retained firmware test ownership. The x64/x86 full build,
58 registered NXVM/integration routes per width, and Core/NXVM boundary gates
pass. No Shared-six or Core source was changed. DeskPro386 is intentionally
deferred to S6 as the final production migration.
