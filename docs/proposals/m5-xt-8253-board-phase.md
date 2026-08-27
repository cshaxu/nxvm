# IBM 5160 8253 PIT Phase Contract

## Purpose

Complete selected XT 8253 PIT clock derivation, channel behavior and IRQ0 route at
the sole Core PIT owner.

## Required sequence

S1 validates original Intel 8253/IBM timing sources. S2 freezes List 1 for each
selected channel/mode, divisor, gate/output, IRQ0, refresh/speaker consumer,
reset and deadline relation. S3 maps List 2 to the current PIT/time owner,
including the sole port ingress, PIT state and output-consumer route. S4
consumes the complete gap batch through that route and proves deadline/order;
it may not add an XT-specific PIT state machine or scheduler path. S5 is the
owner-authorized, bounded current-gate correction discovered before S4
acceptance: it corrects only the retirement-observation test oracle for
unprefixed instructions, retaining the existing Core capture path and making
no PIT or CPU semantic change.

## Boundaries

No wall-clock pacing, RTC behavior, host audio or profile-written PIT state.
