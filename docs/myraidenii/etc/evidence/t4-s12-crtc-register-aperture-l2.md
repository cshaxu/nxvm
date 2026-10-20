# T4 S12 — Video CRTC Register Aperture (L2)

## Basis and scope

The selected-set probe accepted S11's bounded `0x6a0--0x6bf` sprite-protection
upload aperture, then stopped at `0x600` on both host widths.  MAME revision
`8d457b2ecd241b63cab1d9c0c07024365a261398` identifies the Raiden II
`0x600--0x63f` interval as a read/write CRTC receiver.  That address ownership
and read/write classification are L2 behavioral context only: no MAME source,
register definition, table, algorithm, game data or trace was copied.

The admitted model is a reset-cleared 64-byte register aperture owned by
MyRaidenII Video.  Board only translates physical addresses to Video byte
offsets.  It has no second CRTC state and does not turn the wider control
window into RAM.

## Contract

- `0x600--0x63f`: contained read/write blocks preserve their byte order.
- Reset clears all 64 stored bytes.
- `0x5ff` and `0x640`, including the normal Board fallback, retain their
  existing explicit rejection.
- This does not claim physical CRTC timing, layer effects, palette behavior,
  rendering, or a playable Raiden II implementation.

## Verification

The existing Board smoke directly covers invalid Video API arguments, byte
order, the last byte, reset, and both immediate boundaries.  It also keeps the
pre-existing control-window refusal test, with `0x600` correctly removed from
the now-modelled list.

The bounded owner-supplied selected-set probe advanced identically on x64 and
x86 from the prior `0x600` stop to `0x640`:

```text
MYRAIDENII:STARTUP-PROBE:OK reason=7 detail=2147483648 fault-code=1600
executed=71350 ticks=1131500
```

The new stop remains an independently owned Board/Video mapping question.  It
is neither accepted nor described by this S.
