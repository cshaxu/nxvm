# M5 T556 — Five-App Component Layout

Owner admitted T556 on 2026-10-12. The
[proposal](../proposals/m5-app-component-layout.md) owns the requested target
layout and initial S1--S7 plan. S1 is accepted; S2 is active for the MyNES
Machine extraction.

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
