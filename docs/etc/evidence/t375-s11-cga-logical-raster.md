# T375 S11: CGA Logical Raster

`M5:T375:S11:CGA-LOGICAL-RASTER:OK`

## Implemented Contract

The VADP remains the only owner of CGA CRTC state.  In its CGA-only,
non-interlaced route it now accepts and reads R0--R7 and R9, retains R8 as an
unsupported interlace/skew register, and derives its logical state from the
Motorola MC6845 register units:

| Logical state | VADP derivation |
| --- | --- |
| Character clocks per scan line | `R0 + 1` |
| Scan lines per character row | `R9 + 1` |
| Total scan lines | `(R4 + 1) * (R9 + 1) + R5` |
| Displayed scan lines | `R6 * (R9 + 1)` |
| Vertical-sync start | `R7 * (R9 + 1)` |
| Vertical-sync width | 16 logical scan lines, bounded by the raster period |

The status owner reports IBM CGA port `3DAh` bit 0 as safe regeneration-buffer
access outside the configured display region and bit 3 during that logical
vertical-sync window.  Text snapshots use R1/R6 geometry, bounded by the
existing snapshot capacity.  R2/R3 are stored and readable as the documented
horizontal-sync controls; this S has no monitor/waveform or horizontal-sync
pin model.

The MC6845 reset contract clears counters and keeps timing-control registers.
Accordingly, a VADP reset preserves an already programmed CGA CRTC bank,
restarts its logical phase at zero, and suppresses display enable until one
logical frame advances.  The existing synthetic three-window phase remains
only for the unprogrammed compatibility path.  It is neither a character-clock
source nor a physical Model-339 timing assertion.

The [Motorola MC6845 data sheet](https://www.bitsavers.org/components/motorola/_dataSheets/6845.pdf)
is the primary register and reset source.  The IBM CGA adapter reference used
by S8 remains the primary source for `3DAh` bit-0 and bit-3 meanings.  No ROM,
guest medium, firmware image, third-party source, or reference-emulator code
was imported.

## Similar-Issue Sweep

The implementation sweep covered every VADP CRTC index/mask/readback route,
reset route, phase advance, `3DAh` status route, text-snapshot geometry path,
and the adjacent EGA-controller CRTC routes.  A tracked-source search for
`supported_crtc_index|crtc_mask|raster_phase|vadp_status|capture_text_snapshot|vadp_reset`
finds the single VADP owner and its focused tests; no duplicate CGA geometry or
second timer exists.  EGA keeps its pre-existing CRTC gate and synthetic
status route.

## Focused Proof

Fresh build and replays passed:

```text
M5:T266:S3:VADP-TEXT-STATUS:OK
M5:T375:S8:MODEL339-CGA-CLOCK-RECONCILIATION:OK
M5:T375:S11:CGA-LOGICAL-RASTER:OK
current.core-machine-vadp-text-status-smoke: Passed
current.core-machine-ega-controller-port-smoke: Passed
current.core-machine-ega-crtc-boundary-port-smoke: Passed
```

The VADP proof writes/masks/reads R0--R7/R9, confirms R8 remains unreadable,
then proves reset preservation, first-frame blanking, configured display and
horizontal blank status, a logical vertical-sync status, and R1/R6 snapshot
dimensions.  The two EGA replays retain the neighboring controller and CRTC
boundary behavior.

## Retained Boundary

This closes only source-supported logical CRTC behavior.  R8 interlace/skew,
light pen, default CGA register tables, a source-backed character-clock
publisher, CPU/oscillator conversion, monitor waveform, ISA contention/waits,
and Model-339 physical phase/L3 remain later T375 receivers.
