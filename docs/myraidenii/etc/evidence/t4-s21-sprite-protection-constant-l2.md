# M1 T4 S21 — Sprite-Protection Constant Span

## Basis and boundary

After S20, both selected-set probes stopped on a one-byte write at `0x6c4`.
The inherited Raiden II address map identifies `0x6c4--0x6c5` as a write-only
constant supplied with the distinct `0x6c0` sprite-protection receiver.

- Reference: [Raiden II address map](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/seibu/raiden2.cpp)

The map provides address and write-only ownership only.  It does not establish
the constant's value, a protection algorithm, sprite behavior, or a frame
effect.  S21 therefore imports no code, table, algorithm or ROM data.

## Local model

The exact two-byte span is routed through the existing Board write-only
no-op owner.  Contained byte and word writes succeed without retaining state;
reads and inspection remain refused.  A crossing access is rejected by the
existing boundary protection rather than falling back to ordinary RAM.

## Verification

The existing Board smoke covers both contained widths, read/inspect refusal
and the rejected crossing write.  Both selected-set probes advance through the
span and stop identically at the next independent write:

```text
MYRAIDENII:STARTUP-PROBE:OK ... control-physical=1728 ... control-access=1
```

`1728` is `0x6c0`.  That read/write sprite-protection group remains distinct
and is not modeled by S21.  x64 and x86 Board smokes pass.  No sprite
protection, renderer, desktop/RDP or playable-game claim is made.
