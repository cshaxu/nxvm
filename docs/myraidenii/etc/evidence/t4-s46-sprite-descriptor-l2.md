# T4 S46 Sprite Descriptor L2 Boundary

## Scope

This records only the finite format of one buffered eight-byte sprite entry.
It does not qualify graphics decryption, tile-bank resolution, object ordering,
visibility, palette composition, pixels or real-game behavior.

## L2 layout facts

Inspection of the external MAME SEI25x/RISE1x sprite-device implementation,
alongside the locally pinned Raiden II driver revision
`ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253`, establishes these facts:

- every sprite entry contains four little-endian 16-bit words (eight bytes);
- the first word supplies vertical/horizontal flip, one-to-eight tile height
  and width, a two-bit priority and a six-bit colour index;
- the second word is the tile index;
- bit 12 of the third word is a tile-extension bit and its low nine bits are
  the X coordinate; the fourth word's low nine bits are Y;
- the observed signed-position model maps `0x180..0x1ff` to `-128..-1`, while
  `0x000..0x17f` remain non-negative.

These are L2 facts. No external source text, algorithms, data tables or ROM
content is retained or imported.

## Local boundary

`myraidenii_core_sprite_descriptor_decode()` is a stateless copied-value
decoder. Sprite Buffer remains the only RAM/snapshot owner. A later Video
stage must explicitly consume descriptor values and latched graphics; it may
not reach into live RAM or reuse this decoder as a rendering owner.
