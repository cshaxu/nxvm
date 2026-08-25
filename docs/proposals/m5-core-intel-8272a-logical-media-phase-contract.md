# M5 Core Intel 8272A And Logical Media Phase Contract

## Purpose

Close the inseparable Intel 8272A command, DRQ/DMA, IRQ6, result and logical-media
lifecycle as one controller-plus-media task.

## Admission And Dependencies

Consumes PIC and DMA. Intel 8272 original documentation is the selected
controller authority. NEC uPD765 material is comparative only unless a future
immutable controller selection admits it. Admit selected drive personality,
board wiring and logical-media format/error contract before L3 pacing.

## Scope And Completion

Freeze command-to-DRQ, DMA eligibility, terminal count, IRQ/result, Sense,
not-ready/error, reset, detach/cancel and media geometry rows. Prove each with
deterministic command traces and no host-storage latency leakage.

## Boundary

Flux, spindle measurement and oxide behavior are outside L3. Do not split
media error semantics from the FDC phase that consumes them.
