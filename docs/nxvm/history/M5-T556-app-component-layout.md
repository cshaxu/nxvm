# M5 T556 — Five-App Component Layout

Owner admitted T556 on 2026-10-12. The
[proposal](../proposals/m5-app-component-layout.md) owns the requested target
layout and initial S1--S7 plan. S1 is active for design and migration inventory.

Initial source inspection confirms two semantic boundaries require attention:
MyNES driver/media/debug access private hardware state, and DeskPro D4 files mix
register state with board registration and wiring. The three thin PC Apps can
be flattened through responsibility-based names and synchronized references.
No production, test, build or artifact changes have been made by admission.
