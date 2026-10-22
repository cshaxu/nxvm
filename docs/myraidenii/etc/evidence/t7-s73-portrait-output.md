# T7 S73 — Portrait Output

The existing Board retains its native 320x240 raster. The Machine driver now
rotates that raster 90 degrees anticlockwise when publishing the existing
240x320 graphics frame. Native `(x, y)` becomes `(y, 319 - x)`; host scaling
and physical cabinet aspect are separate matters. The retained Machine-owned
scratch buffer keeps capture failure from changing the published slot.

The local MAME Raiden II driver `raiden2.cpp` (SHA-256
`FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A`)
designates `ROT270` for the selected machine. MAME's
[orientation definitions](https://github.com/mamedev/mame/blob/master/src/emu/emucore.h)
define that orientation as 90 degrees anticlockwise. These are L2 references;
no MAME code or data is imported. The existing driver test independently
checks four differently coloured corners, an interior sample, dimensions,
stride, capture failure and sequence preservation. Controls and the Board's
native raster are unchanged.

Both host widths rebuilt the existing Product target and passed the focused
driver unit. Complete repository-only unit suites passed 697/697 on x64
(83.52s) and x86 (82.92s); documentation governance and whitespace pass.
Production changed +13/-5 and the existing test +24/-4. The added 307,200-byte
scratch is reused per Machine, has no second publisher, and avoids modifying
the destination when capture fails. No native desktop, RDP, selected-set
visual, gameplay or release claim follows. The unrelated native MIDI
integration failure recorded in S72 has not been reclassified or rerun here.
