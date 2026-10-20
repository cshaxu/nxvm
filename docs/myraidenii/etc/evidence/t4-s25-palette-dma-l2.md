# M1 T4 S25 — Selected Palette DMA Receiver (L2)

## Basis and scope

S24's bounded owner-set observation established the first rejected DMA trigger
as raw mode `0x0015`, source `0x07c0`, size `0x00ff` and destination `0x0000`.
The source unit is 64 bytes, so the observed source resolves to `0x1f000`.

Permitted external-emulator research is retained as an L2 source only.  The
pinned Raiden II reference maps `0x1f000--0x1ffff` as a 4096-byte palette
aperture and models 2048 palette entries.  No reference implementation, table
or ROM content is imported.  The current implementation therefore accepts
only the complete selected descriptor; it does not infer a general DMA-length
formula from `size`.

## Local contract

COP remains the only DMA scheduler and uses its existing safe point and checked
Core-memory cycle.  Board provides its existing embedded Video object when
constructing COP.  For mode `0x0015` only, COP:

1. requires exactly `source=0x07c0`, `size=0x00ff` and `destination=0`;
2. reads 2048 little-endian words from `0x1f000` through the existing checked
   memory owner into a temporary local array;
3. commits the complete array to the private Video palette only after every
   source read succeeds.

Video owns the committed 2048 raw words and clears them on reset.  It does not
expose bus readback, define RGB bit positions, create a frame, alter the
renderer, or admit modes `0x0014` and `0x0080--0x0087`.  A mismatched
descriptor or failed source read returns the existing failure status and
leaves the prior Video palette unchanged.

## Verification

The existing Board smoke writes a synthetic 2048-word palette to ordinary
machine RAM at `0x1f000`, triggers the exact descriptor and checks first,
second and final words.  It proves a mismatched destination is refused without
overwriting the accepted palette and that reset clears it.  The Video load
boundary separately rejects a partial array without mutation.

Both owner-set startup probes now pass the former mode-`0x0015` rejection and
stop identically at the next independent mode-`0x0014` descriptor:

```text
cop-operation=2 cop-value=20 cop-source=831 cop-size=635
cop-destination=65534 cop-physical=1788 cop-status=2
executed=87920 ticks=1401690
```

The next descriptor is not accepted by this S.  It is a separate tilemap
receiver question.  This record makes no playable-game, RGB-accuracy or
original-hardware-timing claim.
