# M5 PC/AT 8259A Compliance

Complete a manual-led 8259A compliance matrix on the retained master/slave
owner: initialization, cascade, IRR/ISR/IMR, EOI, fixed and rotating priority,
poll, special-mask/SFNM boundaries, edge/level behavior, and spurious IRQ
semantics.  The task consumes T347/T348 producer timing but does not redesign
CPU exception delivery, invent interrupt sources, or claim arbitrary 8259A
board modes without a PC/AT route and focused proof.

