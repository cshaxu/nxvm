# M1 T4: Video

M1 T4 closes the MyRaidenII Core-video delivery. It owns one deterministic
Board-to-Video frame path: immutable fixed-role graphics ingress, palette and
tile state, a latched sprite buffer, independently implemented L2 sprite
decrypt/pixel access, one compositor, and one bounded RGB32 frame-copy API.
It does not create a Machine, Product, Window, second renderer or retained
external payload.

The final S50 selected-set probe supplies the two program lanes and all seven
graphics roles to the existing Board creation API, runs the real V30 path for a
fixed budget, and copies one frame. Both host widths complete without a
terminal CPU/COP/control failure and report the same neutral 320 by 240,
non-black-count and frame-digest observation. A wrong-size role rejects before
execution. Input staging is transient and removed after each run.

External emulator work is recorded as L2 for finite layout, priority and
behavior facts. Project code is independently written; no MAME source body,
blend table, game ROM/PAL byte or generated pixel output is retained. The
remaining sprite blend/shadow fidelity is explicit product debt, not a hidden
T4 exit condition.

The originally written Video exit incorrectly required Machine/Window
publication. That ownership belongs to the later Product/Machine task, which
will consume this sole native frame path for rotation, presentation and input.
T4's revised proposal makes that boundary explicit.

Evidence is retained in `docs/myraidenii/etc/evidence/t4-s1-*` through
`t4-s50-*`; the final runtime receipt is
`t4-s50-selected-set-frame-probe.md`. The associated source deliveries end at
`fa16956d8`.
