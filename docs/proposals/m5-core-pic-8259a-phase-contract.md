# M5 Core PIC 8259A Phase Contract

## Purpose

Define the finite 8259A command, priority, cascade, acknowledgement, EOI,
mask and reset phases through the existing Core IRQ and transaction boundary.

## Admission And Dependencies

Consumes T449. Before implementation, admit the original 8259A manual and the
selected AT cascade wiring. PIT, RTC, KBC, FDC and HDC consume this contract.

## Scope And Completion

Freeze IRR/ISR/IMR, fixed/rotating priority, ICW/OCW, cascade, IRQ visibility,
acknowledgement, EOI, cancellation and cold-reset rows. Prove each row with
command-to-visible-state traces and focused cascade/reset regressions.

## Boundary

INTA pin waveform and unselected wiring are outside L3. Do not add a second
IRQ dispatch path or a controller-specific CPU delivery mechanism.
