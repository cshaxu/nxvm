# T4 S45 Sprite Buffer L2 Boundary

## Scope

This record qualifies only the Board's bounded sprite-RAM publication path.
It does not describe sprite attributes, decryption, pixels, priority, palette
selection or rendering.

## L2 reference facts

Inspection of the locally pinned MAME Raiden II driver revision
`ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253` established these finite board
layout facts:

- the selected main-CPU sprite RAM range is `0x0c000` through `0x0cfff`;
- a write to `0x068e` through `0x068f` transfers the current sprite RAM into
  the display-side buffered copy.

Those facts are recorded as L2. No MAME source text, algorithms, tables or ROM
data is imported, transliterated or retained here.

## Local ownership

`myraidenii_core_sprite_buffer` owns two private 4 KiB arrays: live RAM and a
latched snapshot. It accepts bounded live reads/writes, copies live to latched
on `latch`, and clears both on reset. The Board alone exposes the physical RAM
and trigger routes. Future Video code can consume only the explicit latched
copy capability; it cannot borrow live Board RAM.

The former broad `0x650` object-write aperture no longer owns the trigger.
Reads of the trigger remain an invalid state rather than falling back to
ordinary RAM.

## Verification intent

The sprite-buffer unit test proves latching isolation and reset with synthetic
bytes. The Board-bus test proves that the physical RAM and both trigger bytes
route through the Board provider, retain live readback, reject trigger reads,
and reset the live region. These tests deliberately do not invent a temporary
sprite renderer or expose a test-only Board snapshot getter.
