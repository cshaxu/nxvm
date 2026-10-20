# M1 T4 S20 — Tile-Bank-01 Endpoint

## Basis and boundary

S19's selected-set probe first stopped on a byte write at `0x6cc`.  A temporary
bounded receiver advanced both host widths to the immediately following byte
write at `0x6cd`.  The Raiden II reference map assigns tile-bank-01 only to
`0x6cc`; its inherited map keeps `0x6cd` as an independent write-only no-op.

- Reference: [Raiden II address map](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/seibu/raiden2.cpp)
- Prior local ownership record: `t3-s28-sprite-flags-boundary.md` keeps
  `0x6cc--0x6cd` distinct from the nearby sprite-flag receiver.

This supports two adjacent one-byte L2 write endpoints only.  It supplies no
documented tile, layer, palette, timing or rendering interpretation and
imports no reference implementation material.

## Local model

The Board owns a reset-zero `lib_u8` tile-bank-01 latch at `0x6cc` and a
separate stateless write-only no-op at `0x6cd`.  Reads and inspection remain
on the existing refusal path.  The latch is only a future, evidenced
Video-consumer input.  Core's existing byte-splitting contract remains
unchanged for a wider access spanning those independent endpoints.

## Verification

The existing Board smoke covers the tile-bank byte, adjacent no-op byte,
read/inspect refusal, neighbouring refusal and reset lifecycle.  Both
selected-set probes pass both endpoints and stop identically at a new one-byte
Board write:

```text
MYRAIDENII:STARTUP-PROBE:OK ... control-physical=1732 ... control-access=1
```

`1732` is `0x6c4`; it is deliberately not absorbed by S20.  x64 and x86 Board
smokes pass.  Rendering, palette behavior, desktop/RDP behavior and a
playable-game claim remain outside this S.
