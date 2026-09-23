# T493 S4 IBM 5160 CGA Immediate Repair

`M5:T493:S4:CGA-IMMEDIATE-REPAIR:OK`

S4 consumes every `S4` row in the accepted
[List 2](t493-s3-cga-current-code-gap-list-2.md) at the existing VADP owner.

| List 2 row | Retained path and correction | Proof |
| --- | --- | --- |
| C21 | `t_vadp_data.cga_lightpen_latched` is the standard-CGA latch. `3DCh` presets it, `3DBh` clears it, `core_machine_vadp_read_status` publishes it as `3DAh` bit 1, and `core_machine_vadp_reset` clears it through the sole VADP state reset. The existing Compaq latch stays separate and its selected route is unchanged. | `core-machine-cga-graphics-port-smoke` checks clear, preset, status and reset. All six Model-40 CECG smokes pass. |
| C30 | `core_machine_vadp_high_res_palette` now produces only black and white while video is enabled. The high-resolution capture path no longer treats `3D9h` as a frame-changing input, because it cannot change a black-and-white output. | `core-machine-cga-graphics-port-smoke` verifies both palette entries and confirms a `3D9h` write leaves the high-resolution copied frame unchanged. |

No new port route, renderer state, profile configuration, VADP wrapper, CRTC
bank or timing source was added. C1--C20 and C22--C29, C31--C33 retain the
owners/dispositions in List 2; source-absent physical behavior remains a
boundary rather than an invented correction.

## Verification

- Built and ran `core-machine-cga-graphics-port-smoke`.
- Focused CTest selection passed 5/5: VADP text, VADP text/status, CGA graphics
  port, display composition and XT 5160 profile.
- Compaq isolation selection passed 7/7: the six Model-40 CECG smokes plus the
  CGA graphics port smoke.
- `git diff --check` passed.

The tracked code/test surface is `vadp.c`, `vadp.h` and
`core_machine_cga_graphics_port_smoke.c`: 37 lines added, 18 removed, net +19,
from `git diff --numstat`. The positive state is the one required generic CGA
latch; the removed color comparison/publication eliminates the obsolete
high-resolution color dependency. VADP remains the sole guest-video state and
snapshot owner.
