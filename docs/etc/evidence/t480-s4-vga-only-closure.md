# T480 S4 VGA-only VADP closure

## Result

VADP remains the sole VGA state owner.  DAC state, chain-4 address selection
and Mode 13h frame construction use the existing VADP register banks, planar
VRAM and copied snapshot route; VM and platform own no duplicate video state.

| Ledger row | Final disposition |
| --- | --- |
| V6 DAC | Manual L3: `3C6h`--`3C9h` mask, independent read/write addresses, six-bit RGB components and three-component increment are VADP state. |
| V7 Mode 13h | Manual L3: source-qualified 320x200 sequential 64,000-byte chain-4 pixels publish one 256-entry copied palette/snapshot. |
| V3/V4 interaction | Manual L3: low two A0000h address bits select the existing plane; graphics memory-map selection remains VADP-owned. |
| V8 | Manual L3 only for the selected CPU aperture and fixed Mode-13 frame; no unproven general raster-fetch model is claimed. |
| V9 | L2/unselected: no source-qualified VGA card/profile physical clock reaches the Core timing axis. |
| V10 | Unsupported until S5: no current profile/card selects VGA. |

## Proof

`current.core-machine-ega-external-port-smoke` drives real ports, writes four
successive A0000h bytes, verifies chain-4 plane selection and observes a
320x200x256 copied snapshot.  The complete focused EGA/VGA/display set passed
15/15, and documentation governance passed after the final active-packet check.

`M5:T480:S4:DAC:OK`

`M5:T480:S4:CHAIN4:OK`

`M5:T480:S4:SNAPSHOT:OK`

`M5:T480:S4:REGRESSIONS:OK`
