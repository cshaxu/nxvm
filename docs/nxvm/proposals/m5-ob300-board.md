# M5 Candidate: OmniBook 300 board wiring

Depends on [basic hardware](m5-ob300-hardware.md). Compose the approved CPU,
RAM and devices in a flat MyOB300 board plan. Verify HP address/port decode,
IRQ, DMA, reset, clock, display and input wiring, card window selection and
control registers. Keep HP-specific behavior App-owned with the existing
executor and guest clock.

Use synthetic programs and media fixtures to prove complete signal routes,
reset defaults and accepted versus unmapped transactions. Do not claim the
original firmware boots from a passing synthetic test. Transfer the proved
board contract to [real-ROM machine assembly](m5-ob300-rom-dos.md).
