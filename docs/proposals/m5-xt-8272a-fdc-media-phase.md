# IBM 5160 8272A FDC And Floppy-Media Phase Contract

## Purpose

Complete the selected XT 8272A plus logical floppy-media chain at the single
Core FDC/media owner.

## Required sequence

S1 validates 8272A and IBM selected-drive material. S2 freezes List 1 for all
selected commands, phases, DRQ/DMA2/IRQ6, media geometry, reset/cancel and
documented timing rows. S3 maps List 2 to the FDC/media owner and test matrix.
Implementation consumes the whole selected gap batch without media mirrors.

## Boundaries

No arbitrary drive/controller breadth, host file policy, separate DMA route or
guessed rotational delay.
