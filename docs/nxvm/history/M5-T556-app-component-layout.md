# M5 T556 — Five-App Component Layout

Owner admitted T556 on 2026-10-12. The
[proposal](../proposals/m5-app-component-layout.md) owns the requested target
layout and ordered S plan. S1--S7 are accepted. Owner admitted S8 comprehensive
quality audit on 2026-10-13, with confirmed repairs assigned to S9; T556 remains open.

S8 audit P1 `a28cac691` is accepted after coordinator inspection of its five
documentation files and finding-to-receiver mapping. Static evidence is not
presented as new runtime proof. S9 is admitted for A1--A5; A6 remains a
separate shared-rule Td receiver. S9 implementation and qualification are now
delivered for owner review; T556 is not closed by the agent.

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

S6 is accepted with delivery `68902e24e` and the later NXVM test-layout
correction. Its initial DeskPro directory split retained a boot qualification
gap, explicitly transferred to S7 rather than counted as passed. Accepted S7
`307cf1bd6`/`2df276f8c` repairs and proves unattended installer boot on both
widths. S9 supersedes only the DeskPro directory shape with the owner's flat
layout and repairs S8 findings; it must retain S7's no-auto-input boot result.

S9 implementation is complete. Core registration now uses Core ownership and
relative identities; DeskPro source/units are flat, with atomic D4 attachment
validation; MyNES moved symbols follow Machine ownership. Current authorities
are reconciled separately from the
[retained status archive](M5-T556-retained-status.md), whose contents are retained
with relative-link relocation. The [S9 report](../etc/evidence/t556-s9-quality-repair-delivery.md)
records both-width full 592-unit passes, every admitted integration profile,
ten release artifacts and actual-diff review. MyNES receiving commits are
`7fcd7b1d9` and `c5ca06e85`; NXVM/Shared use distinct target-scoped commits.
Owner acceptance, not these automatic results alone, closes T556.

## NXVM Closure And MyNES Residual Transfer

The owner accepted S11 delivery `6b0cb13e8` and requested a new PC filename/INI
task. S11 closes with 592/592 units and 32/32 PC integrations per width and
eight rebuilt PC EXEs; its evidence retains unqualified phone-client/DPI checks.
The NXVM scope is closed. S10's MyNES input repair is not complete: its active
packet remains in MyNES Current. Diagnostic delivery `3dc750be2` and NXVM
registration `ed75cf572` publish investigation only, not a working input fix.
That explicit receiving packet preserves the residual independently of T557.

### Subsequent S10 Diagnostic Closure

On 2026-10-13 the owner requested S10 closure. The investigation-only delivery
3dc750be2/ed75cf572 is closed with the unresolved short-input repair transferred
to [MyNES TODO](../../mynes/states/TODO.md#rdp-short-input-repair), replacing
the active packet above. No production repair or mobile-RDP success is claimed.
The diagnostic diff and dual-width evidence were reviewed; this closure changes
governance only, retains current EXEs, and admits no new implementation task.
