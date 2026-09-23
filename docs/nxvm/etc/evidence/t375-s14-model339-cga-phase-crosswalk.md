# T375 S14: Model-339 CGA Phase Crosswalk

`M5:T375:S14:MODEL339-CGA-PHASE-CROSSWALK:OK`

## Crosswalk Result

No VADP correction is admitted.  The constrained 86Box-v6.0 IBM-CGA state
model agrees with the primary-source logical behavior already owned by S11:

| Route | VADP / primary logical contract | Qualified 86Box cross-check | Disposition |
| --- | --- | --- | --- |
| CRTC R0--R7/R9 | Register masks and non-interlaced geometry use MC6845 units. | Its CGA state recomputes the phase partition after timing-register writes. | Consistent; no new owner. |
| Display cutoff | Displayed scan lines end at `R6 * (R9 + 1)`. | Display state drops on the R6 row boundary. | Consistent. |
| Horizontal blank | Buffer-safe status begins at R1's character boundary. | Status bit 0 is raised during the off/display-blank segment. | Consistent for the selected logical comparison mask. |
| Vertical sync | Sync begins at `R7 * (R9 + 1)` and spans 16 logical scan lines. | Status bit 3 is asserted at R7 with scanline zero and held for 16 scan lines. | Consistent. |
| Reset/restart | Programmed logical CRTC state is retained while phase restarts. | Reference has its own device lifecycle, not an IBM selected-board reset oracle. | Primary S11 contract retained; no secondary reset claim. |
| R8 interlace/skew | Unsupported and unreadable by the selected VADP route. | The reference retains an interlace/skew mask and optional behavior. | Explicitly excluded; no feature expansion. |
| Mode-control video enable | Presentation gating is outside the selected status/phase contract. | Reference gates rendering, not a new selected-board status source. | No phase change. |

The source is a secondary behavioral cross-check only.  It is not an IBM
board-revision, monitor, snow, contention, wait-state, default-programming, or
physical-waveform authority.  No binary run, whole-machine boot, ROM, guest
media, third-party code, or trace was used or committed.

## Similar-Issue Sweep

The sweep covers every VADP CRTC mask/readback, logical-raster activation,
phase advance, reset, `3DAh` status and text-snapshot route; Model-339's VADP
clock-plan binding; and the neighboring EGA CRTC gates.  It finds one VADP
phase owner and no duplicate timer, reference bridge, or test-only API.

## Verification And Transfer

The S13 Model-339 clock smoke remains the cadence binding/reset proof.
`core-machine-vadp-text-status-smoke` retains the project-owned programmed
CRTC/status checkpoints, and the two EGA smokes guard the adjacent route.

The absence of a code repair does not close board timing.  A runnable reference
trace, exact IBM adapter revision/default programming, ISA contention/waits,
snow, monitor waveform and final Model-339 L3 decision remain in the T375
board/device phase receiver and final 5170 audit.
