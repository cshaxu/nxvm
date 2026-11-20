# M5 T590: OmniBook 300 board wiring

Depends on [basic hardware](m5-ob300-hardware.md). Compose the approved CPU,
RAM and devices in a flat MyOB300 board plan. Verify HP address/port decode,
IRQ, DMA, reset, clock, display and input wiring, card window selection and
control registers. Keep HP-specific behavior App-owned with the existing
executor and guest clock.

| S | Scope and exit evidence |
| --- | --- |
| 1 | Trace original reset and early chipset use, separating the cold and management routes. Establish a bounded register and clock contract from firmware and primary chipset evidence. |
| 2 | Create the flat App-owned board plan, reset and chipset register decode. Verify synthetic CPU, RAM, IRQ, DMA, PIT, RTC and A20 paths, including isolation of unclaimed routes. |
| 3 | Bind the original board's display and input routes to the qualified C&T and AT devices. Verify byte and word I/O, memory aperture, copied frames and guest auxiliary input. |
| 4 | Implement and verify System Card window, bank and attribute/common decode using synthetic images and the sourced HP routing contract. Keep protected assets external. |
| 5 | Qualify the complete board plan on both host widths and hand it to the real-ROM machine task. Preserve an exact inventory of functional ranges and modeled behaviors. |

Use synthetic programs and media fixtures to prove complete signal routes,
reset defaults and accepted versus unmapped transactions. Do not claim the
original firmware boots from a passing synthetic test. Transfer the proved
board contract to [real-ROM machine assembly](m5-ob300-rom-dos.md).

The [original S2 replay](../etc/evidence/t590-s2-ob300-board.md#original-reset-replay)
identifies indexed writes to 2Ch and 2Fh immediately before the firmware
searches below its mapped reset ROM. S4 must verify the lower F000 segment
against the card marker at offset 7F7FF0h and establish how the guest's
register writes select that range. Keep that decode in the HP board.
