# M5 T580 S24 — PC110 EC block-B receiver

## Source and decision

`Open-Source-PC110/Discovery/Pluto/readme.md` records the corrected PC110 EC
port map and captured Block-B index file.  Its checked input hash is:

| Input | SHA-256 |
| --- | --- |
| `Discovery/Pluto/readme.md` | `691DE9101FD0A1AC12E6E9AB9BEB9ABDF7A2507501E208814E322EDAC3E38533` |

The record distinguishes the S23 mailbox at `15E8h`--`15EFh` from Block B at
`35E8h`--`35EFh`.  It establishes `35EAh` as the index port, `35EBh` as the
data port, and a 32-entry register file whose selected index wraps from
`20h`--`3Fh` to `00h`--`1Fh`.  The selected TFT BIOS independently accesses
those same ports in its F000:DCCB helper.

MyPC110 installs one App-local receiver containing only the selected index and
the documented static capture.  It supports an index write and data read; it
does not invent data writes, dynamic AC/battery state, a busy interval, MCU
execution, or a second clock.  This is L2 compatibility from a captured
register image, not a physical power-management model or POST qualification.

## Regression scope

`ec_block_smoke` exercises the ordinary Core route: representative captured
values, the documented index wrap, unsupported data write, and invalid
non-byte index.  Both PC110 construction paths install this one receiver.
The work cannot establish real POST/Easy-Setup progression, panel/power,
storage/guest behavior, lifecycle UX, or original-DSTN support.
