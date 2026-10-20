# M1 T4 S20 — Tile-Bank-01 Endpoint

## Basis and boundary

S19's selected-set probe first stopped on a byte write at `0x6cc`.  A temporary
bounded receiver advanced both host widths to the immediately following byte
write at `0x6cd`.  The Raiden II reference map identifies the combined
`0x6cc--0x6cd` range as a write-only tile-bank-01 endpoint.

- Reference: [Raiden II address map](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/seibu/raiden2.cpp)
- Prior local ownership record: `t3-s28-sprite-flags-boundary.md` keeps
  `0x6cc--0x6cd` distinct from the nearby sprite-flag receiver.

This supports a two-byte L2 endpoint only.  It supplies no documented tile,
layer, palette, timing or rendering interpretation and imports no reference
implementation material.

## Local model

The Board owns a reset-zero `lib_u16` latch over `0x6cc--0x6cd`.  Each byte
lane and a contained word write are accepted; reads and inspection remain on
the existing refusal path.  The retained value is only a future, evidenced
Video-consumer input.  The existing Core byte-splitting contract remains
unchanged for an access spanning independent registers.

## Verification

The existing Board smoke covers both byte lanes, a word write, read/inspect
refusal, neighbouring refusal and reset lifecycle.  Both selected-set probes
pass the complete two-byte endpoint and stop identically at a new one-byte
Board write:

```text
MYRAIDENII:STARTUP-PROBE:OK ... control-physical=1732 ... control-access=1
```

`1732` is `0x6c4`; it is deliberately not absorbed by S20.  x64 and x86 Board
smokes pass.  Rendering, palette behavior, desktop/RDP behavior and a
playable-game claim remain outside this S.
