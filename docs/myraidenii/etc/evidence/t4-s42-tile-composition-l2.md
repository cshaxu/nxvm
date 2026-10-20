# T4 S42 L2 Tile Composition

## Reference And Scope

The owner-approved implementation-reference policy applies. The inspected,
untracked MAME master is revision `ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253`;
the relevant external paths are `src/mame/seibu/raiden2.cpp` and
`src/mame/seibu/raiden2_v.cpp`. No source text, data table, ROM byte, palette
payload or decryption material from that reference enters this repository.

This is an L2 native-frame model, not an original-board accuracy claim.

## Independently Implemented Facts

- the three 16 by 16 tile layers each occupy a 32 by 32 word map;
- the text layer occupies a 64 by 32 word map of 8 by 8 characters;
- a map word has a 12-bit element index and a four-bit palette bank;
- the initial bank mapping is background 0, middle 1, foreground 4 and text 0;
- tilemap word ranges are background 000h--3ffh, foreground 400h--7ffh,
  middle 800h--bffh and text c00h--13ffh;
- layer palette bases are 400h, 420h, 410h and 700h respectively;
- pixel value 15 is transparent; opaque layers compose background, middle,
  foreground then text;
- the native visible destination is 320 by 240 RGB32 pixels.

The implementation expresses those finite facts as arithmetic in the existing
Video owner. It reuses the existing copied graphics bundle, palette and
tilemap. Board only forwards its existing bank latches to the read-only frame
copy operation. No renderer, queue, thread, shared contract or second frame
format exists.

## Deliberate Limits

Sprites remain encrypted and are not decoded or drawn. This packet also does
not model scrolling, layer enable bits, brightness, blending, rotation,
raster-side timing, Machine publication or host presentation. A frame is only
returned after complete bounded composition; an invalid source reports the
existing status and does not publish a partial result.

## Regression

The ROM-free `graphics_smoke` uses synthetic image data. It assigns distinct
tilemap entries and palette groups to all four layers, then removes text,
foreground, middle and background in turn. The sequence proves the two
background inputs are selected by the banked element number, the exact map
regions and palette groups are not interchangeable, transparent pixel 15
preserves the next lower layer, palette-bank lookup produces RGB32, and text
has the final priority. `board_bus_smoke` proves the Board's public frame copy
reaches the single Video owner.

After the corrective mapping review, focused x64/x86 Core tests passed. The
complete repository unit selections also passed on the finished source: x64
ran 642 of 643 under the ordinary sandbox and reran its junction-only cleanup
case successfully under the required authorized Windows context; x86 passed all
643 entries in that context. No external game asset, desktop, RDP or
playable-game claim is part of this evidence.
