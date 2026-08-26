# IBM 5160 8259A Interrupt-Controller Phase Contract

## Purpose

Complete selected XT 8259A board wiring, IRQ publication and phase behavior at
the sole Core PIC owner.

## Required sequence

S1 admits the original 8259A and IBM board sources. S2 creates List 1 for all
selected command/register, IR input, acknowledge/vector, masking, reset and
NMI-adjacent timing rows. S3 maps List 2 to `pic.c` and its callers/tests.
Implementation consumes the entire gap batch through that owner only.

## Boundaries

No AT cascade, ELCR, APIC, profile-side PIC mutation or second dispatch path.
