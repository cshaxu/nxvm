# M1 T4 S27 — Selected Tilemap DMA Receiver (L2)

## Basis and scope

S24 and S26 bounded the first rejected mode-`0x0014` DMA descriptor to
`source=0x033f`, `size=0x027b`, `destination=0xfffe` and `relative=0`.
The source unit is 64 bytes, so its source address is `0x0cfc0`.

Permitted external-emulator research is used only as an L2 behavioral source.
It identifies this selected transfer as a 0x2800-byte tilemap image: exactly
0x1400 raw 16-bit words. No source code, tables, ROM content, tile layout,
scrolling behavior or rendering algorithm is imported.

## Local contract

COP remains the single DMA scheduler and reads the descriptor's relative value
at trigger time, together with its existing selected context. Mode `0x0014`
accepts only the four values above. It reads all 0x1400 little-endian words
through existing checked Core memory into a local candidate, then commits that
complete candidate to the private Video tilemap.

Video owns raw words only. It clears them on reset and has no bus readback,
tile decoder, renderer, frame, palette interpretation, generic DMA length or
other mode acceptance. A descriptor mismatch or failed source read returns the
existing failure status before Video is changed.

## Verification

The existing Board smoke writes a synthetic image at `0x0cfc0`, triggers the
exact descriptor and verifies first, second and final words. It also proves
that direct partial Video input, a mismatched destination and a nonzero
relative value leave the accepted image intact, and that reset clears it.

Both owner-set startup probes advance beyond the former rejected mode-`0x0014`
DMA and reach the next independent Board-control write:

```text
fault-code=1738 cop-operation=2 cop-value=20 cop-source=0 cop-size=0
cop-destination=0 cop-relative=0 cop-physical=0 cop-status=0
control-valid=1 control-physical=1738 control-bytes=1 control-access=1
executed=713508 ticks=11857875
```

The new stop is not interpreted by this S. This record makes no claim of a
playable game, hardware-exact COP timing, tile decoding, frame output or color
accuracy.
