# T4 S41 L2 Graphics Layout

## Reference

The owner authorized implementation-source inspection as an L2 reference on
2026-10-20. The inspected external, untracked source is MAME master revision
`ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253`,
`src/mame/seibu/raiden2.cpp`, acquired outside this repository at
`O:\repos.external\mame-raiden2-l2\raiden2.cpp`.

The source payload and its hashes remain external. No MAME source, data table,
ROM data or decryption material is copied into this repository.

## Adopted L2 Facts

- character images are 8 by 8, four-bit pixels, with 32 bytes per element;
- background images are 16 by 16, four-bit pixels, with 128 bytes per element;
- character/background data is separately decoded from the still-encrypted
  sprite data;
- the selected set has two background image inputs of two MiB each.

`graphics.c` independently expresses just those image-addressing facts as
arithmetic. It has no imported tables, no sprite decoder, no composition order,
no scrolling, and no decryption claim.

## Verification

`graphics_smoke.c` uses synthetic zeroed image buffers with three individually
set bits. It proves pixel extraction from both halves of an 8 by 8 character
and a 16 by 16 background element, together with null and bounds rejection.
It does not use external assets.

Both focused builds and tests passed:

- `gcc-x64-release`: `myraidenii.core/graphics-smoke`;
- `gcc-x86-release`: `myraidenii.core/graphics-smoke`.

## Limits And Next Boundary

This is a decoder primitive only. The next video S must qualify tilemap word
interpretation and layer composition before creating a native frame. Sprite
decryption and sprite composition remain separate L2 work; this change neither
publishes a frame nor claims that RAIDEN II is playable.
