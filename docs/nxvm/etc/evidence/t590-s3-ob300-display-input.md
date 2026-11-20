# T590 S3 OmniBook 300 display and input wiring

The board owns one C&T 65510 instance. Its port, memory and copied
snapshot routes access that instance. Reset goes through the existing
board profile reset phase. The App configuration borrows the setup
through construction and uses the bound profile only while Core owns it.
There is no second VGA controller or host rendering path.

The C&T adapter covers the controller's existing color VGA ports and
legacy A0000h–BFFFFh aperture. The Core byte-lane router handles the
paired sequencer, graphics, CRTC and extension index/data registers.
The C&T chip selects active portions of the aperture and translates
banks. Linear selection declines the legacy range, which currently falls
back to Core RAM. External linear, System Card and HP shadow decode
remain separate board work. The App derives the cursor's nominal 60 Hz
VSync count from existing guest ticks as an L2 model.

The existing AT 8042 still owns keyboard, auxiliary and IRQ routing.
No HP command-extension callback is required by this test. An App
board test runs a guest OUT/IN word pair through 3D6h, inspects the 65510
ID, programs a 640 × 480 frame and writes the first pixel through the
guest aperture. It verifies that selecting the non-legacy mode removes
the VGA route and that a RAM write then leaves the saved VGA pixel
intact. The same test services real IRQ1 and IRQ12 guest handlers,
checks a keyboard byte and AUX enable acknowledgement, and then receives
all three relative report bytes. Both cached widths pass.

Cached unit qualification passed all 717 registrations on each host
width. The x64 aggregate took 295.41 seconds. The x86 aggregate
reported 713 passing registrations before its 300-second deadline.
Three of four follow-ups passed on the first retry. The Lib Types
layout test timed out in the restricted runner and passed in 70.02
seconds when run alone outside that restriction, using the same
build tree, fixture, and 180-second per-test budget.

| Verification record | SHA-256 |
| --- | --- |
| x64 unit aggregate | `64451e77ba39daff03abcf157d4234a1e6c388ff5a4b35e8fe314040249e64c6` |
| x86 unit aggregate | `35e4d51341815c17feccd8e3cf735824d5b3ffc853af21e44b6395ea951a56a4` |
| x86 first follow-up | `19b45d80beef25cffa5483d4cb985d2e443ba2d3b4e0c8ddf8f5b3794dd1cbd0` |
| x86 Types isolation | `655fc927948586e404a5ff80ca8c29db1cd78ae132e45603be3573d28a6d99cd` |

The original VGA option ROM and complete HP POST remain unverified. The
[S2 replay](t590-s2-ob300-board.md#original-reset-replay) identifies
the earlier card-window boundary to resolve before that can be proven.
No public package, owner firmware, media or existing product artifact
changes in this batch.

## System Card handoff

A separate scratch probe exercised original BIOS bytes with a candidate
System Card route. The eight-byte BIOS0 marker matches at guest
F000:7FF0 when card offset 7F0000h supplies the lower F000 segment.
The next little-endian field is 00800000h. The firmware converts it
to a 16 KiB source-page number, then programs 2Ch..2Eh as BC/C2/00
and 2Fh..31h as BD/C2/01. The lower six bits of BC and BD select
F0000h and F4000h in 16 KiB units. Under this candidate decode, both
host widths execute from the mapped second page and stop at F51B4h
on the unimplemented 3Ah write. The subsequent code reads 3Bh
as a status register, so ordinary storage alone cannot establish
that contract. The probe exercised the two common windows only.
Attribute/CIS selection, detailed flag semantics and the physical
decoder width still require evidence or an explicit model.

Both scratch logs have SHA-256
`eb12dbd30edd1f59965bcd5e2f390663bdc9acd9faaa53c5dc209b5d5ccec13a`.
