# M1 T4 S29 — Sprite-protection endpoint latches (L2)

## Basis and limit

S28's selected-set probe stopped at a one-byte write to `0x6d8`. Permitted
L2 address-map research establishes the finite Sprite-protection endpoint
group and its access directions. This record uses only that ownership and
direction evidence; it imports no reference implementation, table, handler,
ROM content, protection algorithm, decryption behavior, or rendering effect.

## Board contract

The existing private Board Sprite-protection latch owner now receives exactly
these little-endian 16-bit pairs, all reset to zero:

| Pair | Direction |
| --- | --- |
| `0x6d8--0x6d9` X | write only |
| `0x6da--0x6db` Y | write only |
| `0x6dc--0x6dd` max-X | read/write |
| `0x6de--0x6df` source | write only |

The pre-existing broad Sprite-object replacement aperture explicitly excludes
this eight-byte group, so these exact routes are its sole owner. The latches
do not alter COP processing, sprite transfer, frame output, timing, or any
public API.

## Verification

- Existing Board bus smoke verifies contained byte/word writes, max-X
  read/inspect, write-only rejection, and reset-zero behavior on x64 and x86.
- Existing selected-set startup probes pass the former `0x6d8` stop on both
  widths and next stop at the separate unreceived read pair `0x762--0x763`.
- Complete repository CTest qualification: 668 regular tests pass on x64 and
  x86; the separately executed junction-based `x86-nxvm.dependency-dag-negative`
  also passes on each width.

No desktop, RDP, ROM payload, media, or playable-game qualification is claimed.
