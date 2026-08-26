# IBM 5160 8254 PIT Phase Contract

## Purpose

Complete selected XT PIT clock derivation, channel behavior and IRQ0 route at
the sole Core PIT owner.

## Required sequence

S1 validates original Intel/IBM timing sources. S2 freezes List 1 for each
selected channel/mode, divisor, gate/output, IRQ0, refresh/speaker consumer,
reset and deadline relation. S3 maps List 2 to the current PIT/time owner.
Implementation consumes the complete gap batch and proves deadline/order.

## Boundaries

No wall-clock pacing, RTC behavior, host audio or profile-written PIT state.
