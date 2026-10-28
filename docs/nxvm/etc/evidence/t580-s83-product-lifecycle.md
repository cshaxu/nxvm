# T580 S83 — actual PC110 product lifecycle

The existing temporary-directory process integration now additionally sends
`start`, `pause`, `resume`, `reset`, `resume`, `stop` and `exit`, and requires
each corresponding lifecycle completion from the actual executable. The test
continues to verify CMOS persistence across separate processes. Both DSTN and
TFT products passed on x86 and x64. The test uses temporary INI and CMOS files;
it does not mutate owner configuration or media.

This product-process path complements the S78/S79 lower-level BIOS diagnostic,
which accesses internal machine state and does not prove that the product
process took its execution route. A monitor completion is not a guest-screen,
physical panel or RDP presentation assertion. The full registered repository
unit suite passed on each architecture, 713/713 per width. No production
source or executable changed in this S.
