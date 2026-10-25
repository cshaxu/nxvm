# M5 T580 S23 — PC110 EC mailbox receiver

## Source and decision

`Open-Source-PC110/Discovery/ULTRACHG/README.md` is a clean-room
disassembly record of the PC110 DOS utility `ULTRACHG.COM`. Its checked input
hash is:

| Input | SHA-256 |
| --- | --- |
| `Discovery/ULTRACHG/README.md` | `3A4E3AC965776B38E41B01715C4F1B7877450E9037E6AB47A9B1EEAC256D83BB` |

It establishes the x86-visible EC mailbox: `15E8h` carries data, `15ECh`
carries command/status, and polling software treats bits 0--1 clear in the
status byte as ready. The source also proves the `31h` command-before-data
transaction used by that utility. It does **not** establish an MCU command
grammar, a mandatory busy interval, an APM return convention, reset semantics
or a real-firmware POST transaction.

MyPC110 therefore installs one App-local mailbox receiver. It reports the
proven ready state and records byte writes. It deliberately does not execute
an MCU image, infer a busy duration, synthesize an APM response, special-case
firmware addresses, or publish a second input/power route. This is L2
compatibility behavior from a documented software trace, not a claim of
physical EC timing or complete power emulation.

## Regression scope

`ec_mailbox_smoke` uses the ordinary Core port route: it verifies ready status,
the observed `31h` command write and a following data byte, while rejecting an
unsupported data read and a non-byte write. Base-composition and construction
tests prove both MyPC110 construction paths retain the receiver without a
second ownership path.

The receiver may enable later real firmware progress only if that firmware
actually reaches the mailbox. It does not by itself prove POST, Easy-Setup,
storage, panel/power, guest boot or the original-DSTN model.
